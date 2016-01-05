/*
 * tunalloc.h
 *
 */
#ifndef _TUNALLOC_H
#define _TUNALLOC_H

//TODO macro for OS identification
//#define _PL_NODE_
 

int tun_alloc_mq(char *dev, int queues, int *fds);
int tun_set_queue(int fd, int enable);

/**
 * \fn int tun_alloc_pl(int iftype, char *if_name)
 * \brief Allocate a tun interface on a PlanetLab
 * VM.
 *
 * \param iftype The interface type (IFFTUN or IFFTAP)
 * \param if_name buffer to be filled with newly created if name.
 * \return fd
 */ 
int tun_alloc_pl(int iftype, char *if_name);

/**
 * \fn int tun_allocl(int iftype, char *if_name)
 * \brief Allocate a tun interface.
 *
 * \param iftype The interface type (IFFTUN or IFFTAP)
 * \param if_name buffer to be filled with newly created if name.
 * \return fd
 */ 
//int tun_alloc(const char *ip, char *dev);

/**
 * \fn char *create_tun_pl(const char *ip, const char *prefix, int nat, int *tun_fds)
 * \brief Allocate and set up a tun interface.
 *
 *    This function is specific to planetlab.
 *
 * \param ip The address of the interface.
 * \param prefix The prefix of the virtual network.
 * \param nat NAT the tun interface or not.
 * \param tun_fds A pointer to an int to be set to the tun interface fd.
 * \return A pointer (malloc) to the interface name.
 */ 
char *create_tun_pl(const char *ip, const char *prefix, int nat, int *tun_fds);

/**
 * \fn char *create_tun(const char *ip, const char *prefix, int nat, int *tun_fds)
 * \brief Allocate and set up a tun interface.
 *
 *    This function is specific to planetlab.
 *
 * \param ip The address of the interface.
 * \param prefix The prefix of the virtual network.
 * \param nat NAT the tun interface or not.
 * \param tun_fds A pointer to an int to be set to the tun interface fd.
 * \return A pointer (malloc) to the interface name.
 */ 
//char *create_tun(const char *ip, const char *prefix, int nat, int *tun_fds);
char *create_tun(const char *ip, char *dev, int *tun_fds);

#endif
