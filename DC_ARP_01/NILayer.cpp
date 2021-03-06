// NILayer.cpp: implementation of the CNILayer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DC_ARP_01.h"
#include "NILayer.h"
#include "EthernetLayer.h"
#include "IPLayer.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CNILayer::CNILayer( char *pName, LPADAPTER *pAdapterObject, int iNumAdapter )
	: CBaseLayer( pName )
{
	m_AdapterObject = NULL;
	m_iNumAdapter = iNumAdapter; 
	m_thrdSwitch = TRUE;
	adapterOpenedSize = 0;
	SetAdapterList(NULL);

}

CNILayer::~CNILayer()
{
}

void CNILayer::PacketStartDriver()
{
	char errbuf[PCAP_ERRBUF_SIZE];

	if(m_iNumAdapter == -1){
		AfxMessageBox("Not exist NICard");
		return;
	}
	
	adapterOpenedList[adapterOpenedSize] = pcap_open_live(m_pAdapterList[m_iNumAdapter]->name,1500,PCAP_OPENFLAG_PROMISCUOUS,2000,errbuf);
	adapterOpenedIndexList[m_iNumAdapter] = adapterOpenedSize++;
	
	if(!adapterOpenedList[adapterOpenedSize]){
		AfxMessageBox(errbuf);
		return;
	}
	/*
	CNILayer newNI("NI");
	newNI.SetAdapterList(NULL);
	newNI.m_AdapterObject = pcap_open_live(m_pAdapterList[m_iNumAdapter]->name,1500,PCAP_OPENFLAG_PROMISCUOUS,2000,errbuf);
	AfxBeginThread(ReadingThread, &newNI);
	*/
}

void CNILayer::startThread()
{
	AfxBeginThread(ReadingThread, this);
}

pcap_if_t *CNILayer::GetAdapterObject(int iIndex)
{
	return m_pAdapterList[iIndex];
}

void CNILayer::SetOpenedAdapterObject(int index)
{
	m_AdapterObject = adapterOpenedList[index];
}

PPACKET_OID_DATA& CNILayer::getNICAddress(int index)
{
	LPADAPTER adapter = PacketOpenAdapter(m_pAdapterList[index]->name);
	
	PPACKET_OID_DATA OidData;
	OidData = (PPACKET_OID_DATA)malloc(sizeof(PACKET_OID_DATA));
	OidData->Oid = 0x01010101;
	OidData->Length = 6;

	PacketRequest(adapter, FALSE, OidData);
	return OidData;
}
void CNILayer::SetAdapterNumber(int iNum)
{
	m_iNumAdapter = iNum;
}

void CNILayer::SetAdapterList(LPADAPTER *plist)
{
	pcap_if_t *alldevs;
	pcap_if_t *d;
	int i=0;
	
	char errbuf[PCAP_ERRBUF_SIZE];

	for(int j=0;j<NI_COUNT_NIC;j++)
	{
		m_pAdapterList[j] = NULL;
	}

	if(pcap_findalldevs(&alldevs, errbuf) == -1)
	{
		AfxMessageBox("Not exist NICard");
		return;
	}
	if(!alldevs)
	{
		AfxMessageBox("Not exist NICard");
		return;
	}

	for(d=alldevs; d; d=d->next)
	{
		m_pAdapterList[i++] = d;
	}
}

BOOL CNILayer::Send(unsigned char *ppayload, int nlength)
{
	for(int i = 0; i < adapterOpenedSize; i++)
	{
		pcap_sendpacket(adapterOpenedList[i],ppayload,nlength);
	}
	/*
	if(pcap_sendpacket(m_AdapterObject,ppayload,nlength))
	{
		AfxMessageBox("패킷 전송 실패");
		return FALSE;
	}*/
	return TRUE;
}

BOOL CNILayer::Receive( unsigned char* ppayload, int adapter_number )
{
	BOOL bSuccess = FALSE;
	SetOpenedAdapterObject(adapter_number);

	int n=0;
	for(int i = 0; i < MAX_ADAPTER_COUNT; i++)
	{
		if(adapterOpenedIndexList[i] == adapter_number)
		{
			n = i;
			break;
		}
	}
	((CEthernetLayer*)GetUpperLayer(0))->setNICCard(n);

	CIPLayer::PIPLayer_HEADER pFrame = (CIPLayer::PIPLayer_HEADER) ppayload;
	bSuccess = mp_aUpperLayer[0]->Receive(ppayload);
	return bSuccess;
}
/*
BOOL CNILayer::Receive( unsigned char* ppayload )
{
	BOOL bSuccess = FALSE;
	((CEthernetLayer*)GetUpperLayer(0))->setNICCard(m_iNumAdapter);

	CIPLayer::PIPLayer_HEADER pFrame = (CIPLayer::PIPLayer_HEADER) ppayload;
	bSuccess = mp_aUpperLayer[0]->Receive(ppayload);
	return bSuccess;
}
*/
UINT CNILayer::ReadingThread(LPVOID pParam)
{
	//////////////////////// fill the blank ///////////////////////////////
	struct pcap_pkthdr* header;
	const u_char* pkt_data;
	int result; 
	int num_adapter = (int)pParam;

	CNILayer* pNI = (CNILayer*) pParam; 

	int index = 0;
	while(pNI->m_thrdSwitch)
	{
		result = pcap_next_ex(pNI->adapterOpenedList[index % pNI->adapterOpenedSize],&header,&pkt_data);
		//result = pcap_next_ex(pNI->m_AdapterObject,&header,&pkt_data);

		if(result == 0)
		{
		}

		else if(result == 1)
		{
			pNI->Receive((u_char*)pkt_data, index % pNI->adapterOpenedSize);
			//pNI->Receive((u_char*)pkt_data);
		}
		else if(result < 0)
		{
		}
		index++;
	}
	return 0;
	///////////////////////////////////////////////////////////////////////
}

UINT CNILayer::FileTransferThread(LPVOID pParam)
{
	CNILayer *pNI = (CNILayer *)pParam;

	return 0;
}