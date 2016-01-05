/**
 * \file peer.c
 * \brief The fullmesh peer implementation.
 * \author k.edeline
 * \version 0.1
 */

#include "peer.h"
#include "state.h"
#include "destruct.h"

/**
 * \var static volatile int loop
 * \brief The server loop guardian.
 */
static volatile int loop;

/**
 * \fn static void int_handler(int sig)
 * \brief Callback function for SIGINT catcher.
 *
 * \param sig Ignored
 */ 
static void peer_shutdown(int sig);

static void tun_peer_aux(struct arguments *args);
static void tun_peer_pl(struct arguments *args);
static void tun_peer_fbsd(struct arguments *args);

/**
 * \fn static void tun_serv_in(int fd_udp, int fd_tun, struct tun_state *state, char *buf)
 * \brief Forward a packet in the tunnel.
 *
 * \param fd_udp The udp socket fd.
 * \param fd_tun The tun interface fd.
 * \param state The state of the server.
 * \param buf The buffer.
 */ 
static void tun_peer_in(int fd_tun, int fd_cli, int fd_serv, struct tun_state *state, char *buf);

/**
 * \fn static void tun_serv_out(int fd_udp, int fd_tun, struct arguments *args, struct tun_state *state, char *buf)
 * \brief Forward a packet out of the tunnel.
 *
 * \param fd_udp The udp socket fd.
 * \param fd_tun The tun interface fd.
 * \param args The arguments of the server.
 * \param state The state of the server.
 * \param buf The buffer.
 */ 
static void tun_peer_out_cli(int fd_udp, int fd_tun, struct arguments *args, struct tun_state *state, char *buf);

/**
 * \fn static void tun_serv_out(int fd_udp, int fd_tun, struct arguments *args, struct tun_state *state, char *buf)
 * \brief Forward a packet out of the tunnel.
 *
 * \param fd_udp The udp socket fd.
 * \param fd_tun The tun interface fd.
 * \param args The arguments of the server.
 * \param state The state of the server.
 * \param buf The buffer.
 */ 
static void tun_peer_out_serv(int fd_udp, int fd_tun, struct arguments *args, struct tun_state *state, char *buf);

void peer_shutdown(int sig) { loop = 0; }

void tun_peer(struct arguments *args) {
   if (args->planetlab)
      tun_peer_pl(args);
   else if (args->freebsd)
      tun_peer_fbsd(args);
   else
      tun_peer_aux(args);
}

void tun_peer_in(int fd_tun, int fd_cli, int fd_serv, struct tun_state *state, char *buf) {
   int recvd=xread(fd_tun, buf, __BUFFSIZE);
   debug_print("recvd %db from tun\n", recvd);

   if (recvd > 32) {

      struct tun_rec *rec = NULL; 
      //read sport for clients mapping
      int dport = (int) ntohs( *((uint16_t *)(buf+22)) ); // 26 with PI

      /* cli */
      if (dport == state->tcp_port) {
         // lookup initial server database from file 
         in_addr_t priv_addr = (int) *((uint32_t *)(buf+16));
         debug_print("%s\n", inet_ntoa((struct in_addr){priv_addr}));
         /* lookup private addr */
         if ( (rec = g_hash_table_lookup(state->cli, &priv_addr)) ) {
            debug_print("priv addr lookup: OK\n");

            int sent = xsendto(fd_cli, rec->sa, buf, recvd);
            debug_print("wrote %db to udp\n",sent);

         } else {
            errno=EFAULT;
            die("cli lookup");
         }

      /* serv */
      } else if ((rec = g_hash_table_lookup(state->serv, &dport))) {   
         debug_print("dport lookup: OK\n");

         int sent = xsendto(fd_serv, rec->sa, buf, recvd);
         debug_print("wrote %db to udp\n",sent);
      } else {
         errno=EFAULT;
         die("serv lookup");
      }
   } 
}

void tun_peer_out_cli(int fd_udp, int fd_tun, struct arguments *args, 
                      struct tun_state *state, char *buf) {
   struct tun_rec *nrec = init_tun_rec();
   int recvd = 0;
   if ( (recvd=xrecvfrom(fd_udp, (struct sockaddr *)nrec->sa, &nrec->slen, buf, __BUFFSIZE)) < 0) {
      /* recvd ICMP msg */
      //xfwerr(fd_udp, buf,  __BUFFSIZE, fd_tun, state);
      xrecverr(fd_udp, buf,  __BUFFSIZE);
   } else {
      debug_print("recvd %db from udp\n", recvd);

      if (recvd > 32) {
         int sent = xwrite(fd_tun, buf, recvd);

         debug_print("wrote %d to tun\n", sent);     
      } else debug_print("recvd empty pkt\n");
   }
   free_tun_rec(nrec);
}

void tun_peer_out_serv(int fd_udp, int fd_tun, struct arguments *args, 
                       struct tun_state *state, char *buf) {
   struct tun_rec *nrec = init_tun_rec();
   int recvd = 0;
   recvd=xrecvfrom(fd_udp, (struct sockaddr *)nrec->sa, &nrec->slen, buf, __BUFFSIZE);

   debug_print("recvd %db from udp\n", recvd);

   if (recvd > 32) {
      struct tun_rec *rec = NULL;
      int sport           = ntohs(((struct sockaddr_in *)nrec->sa)->sin_port);
      int sent            = 0;
      if ( (rec = g_hash_table_lookup(state->serv, &sport)) ) {
         sent = xwrite(fd_tun, buf, recvd);
         free_tun_rec(nrec);
      } else if (g_hash_table_size(state->serv) <= state->fd_lim) { 
         sent = xwrite(fd_tun, buf, recvd);

         //add new record to lookup tables  
         nrec->sport = sport;
         g_hash_table_insert(state->serv, &nrec->sport, nrec);
         debug_print("serv: added new entry: %d\n", sport);
      } else {
         free_tun_rec(nrec);
         errno=EUSERS; //no need to exit but safer
         die("socket()");
      }
      debug_print("serv: wrote %d to tun\n", sent);     
   } else debug_print("recvd empty pkt\n");

}

void tun_peer_aux(struct arguments *args) {
   /*
sudo  ./src/udptun -f -v -t 1 --udp-daddr=139.165.223.57 --udp-sport=34501 --udp-dport=5001 --tcp-saddr=192.168.2.2 --tcp-sport=34501 --tcp-dport=9876 --tcp-daddr=192.168.2.1
   */

   int fd_tun = 0, fd_serv = 0, fd_cli = 0;
   int fd_max = 0, sel = 0;
   struct sockaddr_in *udp_addr = NULL, *tcp_addr = NULL;
   
   /* init state */
   struct tun_state *state = init_tun_state(args);

   /* create tun if and sockets */
   args->if_name  = create_tun(state->private_addr, NULL, &fd_tun);   
   fd_serv  = udp_sock(state->udp_port);
   fd_cli   = udp_sock(state->port);

   /* run server */
   debug_print("running serv ...\n");  
   pthread_t thread_id;
   if (pthread_create(&thread_id, NULL, serv_thread, (void*) state) < 0) 
      die("pthread_create");
   set_pthread(thread_id);

   /* run client */
   debug_print("running cli ...\n");  
   if (pthread_create(&thread_id, NULL, cli_thread, (void*) state) < 0) 
      die("pthread_create");
   set_pthread(thread_id);

   /* init select main loop */
   fd_set input_set;
   struct timeval tv;
   char buf[__BUFFSIZE];
   fd_max = max(max(fd_cli, fd_tun),fd_serv);
   loop   = 1;
   signal(SIGINT, serv_shutdown);

   while (loop) {
      //build select list
      FD_ZERO(&input_set);
      FD_SET(fd_cli,  &input_set);
      FD_SET(fd_serv, &input_set);
      FD_SET(fd_tun,  &input_set);

      tv.tv_sec  = state->inactivity_timeout; 
      tv.tv_usec = 0;

      sel = select(fd_max+1, &input_set, NULL, NULL, &tv);  
      if (sel < 0) die("select");
      else if (sel == 0) {
         debug_print("timeout\n"); 
         break;
      } else if (sel > 0) {
         if (FD_ISSET(fd_tun, &input_set))      
            tun_peer_in(fd_tun, fd_cli, fd_serv, state, buf);
         if (FD_ISSET(fd_cli, &input_set)) 
            tun_peer_out_cli(fd_cli, fd_tun, args, state, buf);
         if (FD_ISSET(fd_serv, &input_set)) 
            tun_peer_out_serv(fd_serv, fd_tun, args, state, buf);
      }
   }

   close(fd_cli);close(fd_serv);
   close(fd_tun);free_tun_state(state);
   free(args->if_name);
}

void tun_peer_fbsd(struct arguments *args) {

}

void tun_peer_pl(struct arguments *args) {

}

