#ifndef GS_TCP_H
#define GS_TCP_H

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

#include "gs_em_ip_prot.h"

#define TCP_READ_TIMEOUT (-2)

#define TCP_SEND_MSG_TAG_RESPONSE	(0x00)
#define TCP_SEND_MSG_TAG_HEARTBEAT	(0x01)
#define TCP_SEND_MSG_TAG_POSTBACK	(0x02)

#define TCP_SEND_MSG_TYPE (0x01)

#define TCP_RECV_MSG_TYPE_HEARTBEAT_RESPONSE	(0x01)
#define TCP_RECV_MSG_TYPE_DEV_DATA	(0x02)

typedef struct {
	GS_EM_IP_PROT_NET_ADDR_S	m_ServAddr;

	GS_HANDLE						m_TcpSndMsgQ;
	GS_HANDLE						m_TcpRecvMsgQ;
} GS_TCP_INIT_PARAM_S;
GS_HANDLE GS_TCP_Create(GS_TCP_INIT_PARAM_S *pInitParam);
GS_VOID GS_TCP_Destroy(GS_HANDLE Handle);
GS_BOOL GS_TCP_GetConnStat(GS_HANDLE Handle);
GS_VOID GS_TCP_SetConnStat(GS_HANDLE Handle, GS_BOOL IsConnOk);
GS_VOID GS_TCP_ReconfigServerAddr(GS_HANDLE Handle, GS_EM_IP_PROT_NET_ADDR_S *pServerAddr);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* GS_TCP_H */
/* EOF */
