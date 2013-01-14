
#include "network.h"
#include "client.h"

#if defined(__MSC_VER)
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>

#ifndef INVALID_SOCKET
#define INVALID_SOCKET (-1)
#endif
#define SOCKET_ERROR (-1)
#define closesocket(fd) close(fd)
#endif

using namespace ADDON;

bool in_ether (const char *bufp, unsigned char *addr)
{
  if (strlen(bufp) != 17)
    return false;

  char c;
  const char *orig;
  unsigned char *ptr = addr;
  unsigned val;

  int i = 0;
  orig = bufp;

  while ((*bufp != '\0') && (i < 6))
  {
    val = 0;
    c = *bufp++;

    if (isdigit(c))
      val = c - '0';
    else if (c >= 'a' && c <= 'f')
      val = c - 'a' + 10;
    else if (c >= 'A' && c <= 'F')
      val = c - 'A' + 10;
    else
      return false;

    val <<= 4;
    c = *bufp;
    if (isdigit(c))
      val |= c - '0';
    else if (c >= 'a' && c <= 'f')
      val |= c - 'a' + 10;
    else if (c >= 'A' && c <= 'F')
      val |= c - 'A' + 10;
    else if (c == ':' || c == '-' || c == 0)
      val >>= 4;
    else
      return false;

    if (c != 0)
      bufp++;

    *ptr++ = (unsigned char) (val & 0377);
    i++;

    if (*bufp == ':' || *bufp == '-')
      bufp++;
  }

  if (bufp - orig != 17)
    return false;

  return true;
}

bool Network::WakeOnLan(const char* mac)
{
  int i, j, packet;
  unsigned char ethaddr[8];
  unsigned char buf [128];
  unsigned char *ptr;

  // Fetch the hardware address
  if (!in_ether(mac, ethaddr))
  {
    XBMC->Log(LOG_ERROR, "%s - Invalid hardware address specified (%s)", __FUNCTION__, mac);
    return false;
  }

  // Setup the socket
  if ((packet = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
  {
    XBMC->Log(LOG_ERROR, "%s - Unable to create socket (%s)", __FUNCTION__, strerror (errno));
    return false;
  }

  // Set socket options
  struct sockaddr_in saddr;
  saddr.sin_family = AF_INET;
  saddr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
  saddr.sin_port = htons(9);

  unsigned int value = 1;
  if (setsockopt (packet, SOL_SOCKET, SO_BROADCAST, (char*) &value, sizeof( unsigned int ) ) == SOCKET_ERROR)
  {
    XBMC->Log(LOG_ERROR, "%s - Unable to set socket options (%s)", __FUNCTION__, strerror (errno));
    closesocket(packet);
    return false;
  }

  // Build the magic packet (6 x 0xff + 16 x MAC address)
  ptr = buf;
  for (i = 0; i < 6; i++)
    *ptr++ = 0xff;

  for (j = 0; j < 16; j++)
    for (i = 0; i < 6; i++)
      *ptr++ = ethaddr[i];

  // Send the magic packet
  if (sendto (packet, (char *)buf, 102, 0, (struct sockaddr *)&saddr, sizeof (saddr)) < 0)
  {
    XBMC->Log(LOG_ERROR, "%s - Unable to send magic packet (%s)", __FUNCTION__, strerror (errno));
    closesocket(packet);
    return false;
  }

  closesocket(packet);
  XBMC->Log(LOG_NOTICE, "%s - Magic packet send to '%s'", __FUNCTION__, mac);
  return true;
}
