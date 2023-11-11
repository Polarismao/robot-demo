#include "ObjPublic.h"
#include "ObjCan.h"

/*************************************************************
*@brief【描述】
*@author mdq
*@date   2023-11-09
*@note 【备注】
*************************************************************/
void Cf_Can_ParaInit(void)
{


}

/*************************************************************
*@brief【描述】
*@author mdq
*@date   2023-11-09
*@note 【备注】
*************************************************************/
void Cf_CanMainDeal(void)
{
	unsigned short i,m,us_sendCycle;
	CAN_FRAME2515 st_rcecFrame;
    
	for(m=0;m<4;m++)
	{
		if(uc_phyCan_rxFlag[m])
		{
			st_cf_rcecFrame.COB_ID = ul_phyCan_rxId[m];
			st_cf_rcecFrame.uc_exId = uc_phyCan_exId[m];
			st_cf_rcecFrame.DLC = uc_phyCan_rxLen[m];
			for(i=0;i<st_cf_rcecFrame.DLC;i++)
			{
				st_cf_rcecFrame.Data[i] = uc_phyCan_rxMessage[m][i];
			}
			uc_phyCan_rxFlag[m] = 0;
			st_rcecFrame.COB_ID = ul_phyCan_rxId[m];
			st_rcecFrame.DLC = uc_phyCan_rxLen[m];
			st_rcecFrame.uc_exId = uc_phyCan_exId[m];  //传递扩展帧标识符
			for(i=0;i< st_rcecFrame.DLC;i++)
			{
				st_rcecFrame.Data[i] = uc_phyCan_rxMessage[m][i];
			}
		}
	}
	us_cf_canSendTimer += CF_2MS;
    us_sendCycle = CF_10MS;
    
	if(us_cf_canSendTimer>=us_sendCycle)
	{
		us_cf_canSendTimer=0;
		Cf_CanEvtChk();
		Cf_CanTxDeal();
		Cf_CanMachineTestSend();
		if(us_cf_needSend)
		{
			us_cf_needSend = 0;
			physical_can_deal(st_cf_sendFrame.COB_ID,st_cf_sendFrame.uc_exId,st_cf_sendFrame.DLC,st_cf_sendFrame.Data);
		}
	}
	Cf_CanResetDeal();
	Cf_CanCommQuality();
}

/*************************************************************
*@brief【描述】
*@author mdq
*@date   2023-11-09
*@note 【备注】
*************************************************************/
void Cf_CanResetDeal(void)
{
	Cf_CopOnlineJudge();
}

/*************************************************************
*@brief【描述】
*@author mdq
*@date   2023-11-09
*@note 【备注】
*************************************************************/
void Cf_CanCommQuality(void)
{
	signed short s_qualityTemp;
	us_cfCan_qualityChkTimer += CF_2MS;
	if(us_cfCan_qualityChkTimer >= CF_1S)
	{
		s_qualityTemp = 9 - us_cfCan_nmtReceNum;
		if(s_qualityTemp < 0)
			s_qualityTemp = 0;
		us_cfCan_nmtReceNum = 0;
		us_cfCan_qualityChkTimer = 0;
		obj_can1_comm_quality = s_qualityTemp;
	}
}

/*************************************************************
*@brief【描述】
*@author mdq
*@date   2023-11-09
*@note 【备注】
*************************************************************/
void Cf_CanStdRxDeal(void)
{
	Cf_FdMainCopRxDeal();		
}

/*************************************************************
*@brief【描述】
*@author mdq
*@date   2023-11-09
*@note 【备注】
*************************************************************/
void Cf_CanExtRxDeal(void)
{
    //ExtCanID:|-funId--|--endFlag--|--FrameId--|--cmd/srcAddr---|--dstAddr--|
    //ExtCanID:|--4bit--|----1bit---|---8bit----|------8bit------|---8bit----|
    unsigned char uc_funId,uc_endFrameFlag,uc_frameID,uc_cmd,uc_srcAddr,uc_addr;
    unsigned char uc_iapDatalen;
    unsigned char uc_iapSpiDatalenTmp;
    CAN_ID_FRAME st_iapDataId;

    unsigned char uc_mc2mIaptmp = 0;
    unsigned char uc_rvIaptmp = 0;
    unsigned short us_spiBufIndex = 0;

    uc_funId = (unsigned char)((st_cf_rcecFrame.COB_ID>>25) & 0x0f);
    uc_endFrameFlag = (unsigned char)((st_cf_rcecFrame.COB_ID>>24) & 0x01);
    uc_frameID = (unsigned char)((st_cf_rcecFrame.COB_ID>>16) & 0xff);
    uc_cmd = (unsigned char)((st_cf_rcecFrame.COB_ID>>8) & 0xff);
    uc_srcAddr = (unsigned char)((st_cf_rcecFrame.COB_ID>>8) & 0xff);
    uc_addr = (unsigned char)(st_cf_rcecFrame.COB_ID & 0xff);
	
    if(st_cf_rcecFrame.COB_ID==MPC1_RX_ADD_1)
    {
        CF_Mpc1RxDealAdd();
    }
}

/*************************************************************
*@brief【描述】
*@author mdq
*@date   2023-11-09
*@note 【备注】
*************************************************************/
void Cf_CanEvtChk(void)
{
	Cf_EvtChkDoorInit();
    FunMpc1_EvtChkMpc1Gen1Eid();
}

/*************************************************************
*@brief【描述】
*@author mdq
*@date   2023-11-10
*@note 【备注】
*************************************************************/
void Cf_TxIdCycle(void)	 //20ms进每个case的执行为200ms周期
{
    static unsigned char doorCommandToggle=0;
    static unsigned char doorCommandToggle2=0;//A01.7-FR011
	if(st_cf_cycleTxId.ul_funId)
		return;

	switch(us_cf_cycleStep)
	{
		case 0:
		{// 显示类广播帧
			st_cf_cycleTxId.ul_funId = 0x500;
			st_cf_cycleTxId.us_subId = 0x40;
			st_cf_cycleTxId.us_doorType=FBDOOR;//不区分前后门	
			break;	
		}
		case 1:
		{// 显示类广播帧
			us_cf_stepToggle2^=0x01;
			if(us_cf_stepToggle2&0x01)
				st_cf_cycleTxId.ul_funId = 0x740;
			else
				st_cf_cycleTxId.ul_funId = 0x500;
			st_cf_cycleTxId.us_subId = 0x47;
			st_cf_cycleTxId.us_doorType=FBDOOR;
			break;				
		}
		case 2:
		{// car fan control and  bypass run
			st_cf_cycleTxId.ul_funId = 0x100;
			st_cf_cycleTxId.us_subId = 0x71;
			st_cf_cycleTxId.us_doorType=FBDOOR;
			break;		
		}
		case 3:
		{// 显示盘之类
			if(us_cf_cycleStepMonitor==0)
			{
				st_cf_cycleTxId.ul_funId = 0x740;
				st_cf_cycleTxId.us_subId = 0xA0;
				st_cf_cycleTxId.us_doorType=FBDOOR;
			}
			else if(us_cf_cycleStepMonitor==1)//A01.1-R52
			{
				st_cf_cycleTxId.ul_funId = 0x600;
				st_cf_cycleTxId.us_subId = 0x40;
				st_cf_cycleTxId.us_doorType=FBDOOR;//不区分前后门	
			}
			else if(us_cf_cycleStepMonitor==2)//A01.1-R52
			{
				st_cf_cycleTxId.ul_funId = 0x680;
				st_cf_cycleTxId.us_subId = 0;
				st_cf_cycleTxId.us_doorType=FBDOOR;//不区分前后门
			}
			us_cf_cycleStepMonitor++;
			if(us_cf_cycleStepMonitor>2)
				us_cf_cycleStepMonitor = 0;
			break;		
		}
		case 4:	
		{//  模拟量称重			
			st_cf_cycleTxId.ul_funId = 0x741;
			st_cf_cycleTxId.us_subId = 0x41;
			st_cf_cycleTxId.us_doorType=FBDOOR;
			break;
		}
		case 5:
		{// buzzer and overload
			st_cf_cycleTxId.ul_funId = 0x100;
			st_cf_cycleTxId.us_subId = 0x70;
			st_cf_cycleTxId.us_doorType=FBDOOR;
			break;
		}
		case 6:
		{
			us_cf_stepToggle1^=0x01;
			if(us_cf_stepToggle1&0x01)
			{// 显示类
				st_cf_cycleTxId.ul_funId = 0x740;
				st_cf_cycleTxId.us_subId = 0x48;
				st_cf_cycleTxId.us_doorType=FBDOOR;
			}	
			else
			{// 机房IO扩展板
				if(us_cf_cycleStepMc2m==0)   //A02.3-CR028
				{
				    if(FIRST_BRAND)
                        st_cf_cycleTxId.ul_funId = 0x1fff;
             		else
                        st_cf_cycleTxId.ul_funId = 0x5fff;
					st_cf_cycleTxId.us_subId = 0x01;
					st_cf_cycleTxId.us_doorType=FBDOOR;
				}
				else if(us_cf_cycleStepMc2m==1)
				{
				if(FIRST_BRAND)
					st_cf_cycleTxId.ul_funId = 0x1fff;
				else
                        st_cf_cycleTxId.ul_funId = 0x5fff;
					st_cf_cycleTxId.us_subId = 0x02;
				st_cf_cycleTxId.us_doorType=FBDOOR;					
				}
				else if(us_cf_cycleStepMc2m==2)
				{
				    if(FIRST_BRAND)
                        st_cf_cycleTxId.ul_funId = 0x1fff;
             		else
                        st_cf_cycleTxId.ul_funId = 0x5fff;
					st_cf_cycleTxId.us_subId = 0x03;
					st_cf_cycleTxId.us_doorType=FBDOOR;//不区分前后门
				}
				us_cf_cycleStepMc2m++;
				if(us_cf_cycleStepMc2m>2)
					us_cf_cycleStepMc2m = 0;
				break;				
			}				
			break;
		}
		case 7:
		{
			if(function.f_code.door_motor_nub==1)
			{
				if(us_cf_cycleStep4==0)//  1s周期
				{
					if(un_cfCan_fdInit.bit.b_gotInitFinished)
					{	
						if(FIRST_BRAND)
						{
							st_cf_cycleTxId.ul_funId = 0x742;//门机开关门命令
                            if(doorCommandToggle)
                            {
                                doorCommandToggle=0;
 							    st_cf_cycleTxId.us_subId = 0x50;                               
                            }
                            else
                            {
                                doorCommandToggle=1;   
							st_cf_cycleTxId.us_subId = 0x20;//A01.5-FR031 事件帧，更改为1s发送一帧，贯通门时，2S发送，针对蒂森门机不处理周期帧命令更改。
                            }
							st_cf_cycleTxId.us_doorType=FDOOR;
						}
						else
						{
							st_cf_cycleTxId.ul_funId = 0x752;//门机开关门命令
                            if(doorCommandToggle)
                            {
                                doorCommandToggle=0;
 							    st_cf_cycleTxId.us_subId = 0x50;                               
                            }
                            else
                            {
                                doorCommandToggle=1;   
							st_cf_cycleTxId.us_subId = 0x20;//A01.5-FR031
                            }
							st_cf_cycleTxId.us_doorType=FDOOR;				
						}
					}
				}
                else if(us_cf_cycleStep4==2)
                {// 更改为1s周期帧发送。否则，IO门机处理方式，在通讯丢帧时，会报E56问题
    				st_cf_cycleTxId.ul_funId = 0x100;      
                    st_cf_cycleTxId.us_subId = 0x73;
                    st_cf_cycleTxId.us_doorType=FBDOOR;		
                }
				us_cf_cycleStep4++;
				if(us_cf_cycleStep4>=5)
					us_cf_cycleStep4=0;
			}
			else if(function.f_code.door_motor_nub==2)
			{
				if(us_cf_cycleStep5==0)
				{
					if(un_cfCan_fdInit.bit.b_gotInitFinished)
					{
						if(FIRST_BRAND)
						{
							st_cf_cycleTxId.ul_funId = 0x742;//门机开关门命令
                            if(doorCommandToggle)
                            {
                                doorCommandToggle=0;
 							    st_cf_cycleTxId.us_subId = 0x50;                               
                            }
                            else
                            {
                                doorCommandToggle=1;   
							st_cf_cycleTxId.us_subId = 0x20;//A01.5-FR031
                            }
							st_cf_cycleTxId.us_doorType=FDOOR;
						}
						else
						{
							st_cf_cycleTxId.ul_funId = 0x752;//门机开关门命令
                            if(doorCommandToggle)
                            {
                                doorCommandToggle=0;
 							    st_cf_cycleTxId.us_subId = 0x50;                               
                            }
                            else
                            {
                                doorCommandToggle=1;   
							st_cf_cycleTxId.us_subId = 0x20;//A01.5-FR031
                            }							
							st_cf_cycleTxId.us_doorType=FDOOR;						
						}
					}
				}
				else if(us_cf_cycleStep5==1)
				{
					if(un_cfCan_bdInit.bit.b_gotInitFinished)
					{
						if(FIRST_BRAND)
						{
							st_cf_cycleTxId.ul_funId = 0x742;//门机开关门命令
                            if(doorCommandToggle2)//A01.7-FR011
                            {
                                doorCommandToggle2=0;
 							    st_cf_cycleTxId.us_subId = 0x50;                               
                            }
                            else
                            {
                                doorCommandToggle2=1;   
								st_cf_cycleTxId.us_subId = 0x20;//A01.5-FR031
                            }						
							st_cf_cycleTxId.us_doorType=BDOOR;
						}
						else
						{
							st_cf_cycleTxId.ul_funId = 0x752;//门机开关门命令
                            if(doorCommandToggle2)//A01.7-FR011
                            {
                                doorCommandToggle2=0;
 							    st_cf_cycleTxId.us_subId = 0x50;                               
                            }
                            else
                            {
                                doorCommandToggle2=1;   
								st_cf_cycleTxId.us_subId = 0x20;//A01.5-FR031
                            }						
								st_cf_cycleTxId.us_doorType=BDOOR;						
						}
					}
				}
				else if(us_cf_cycleStep5==2)
                {//更改为1s周期帧发送，否则容易引起丢帧引起E56
    				st_cf_cycleTxId.ul_funId = 0x100;      
                    st_cf_cycleTxId.us_subId = 0x73;
                    st_cf_cycleTxId.us_doorType=FBDOOR;		
                }
				us_cf_cycleStep5++;
				if(us_cf_cycleStep5>=5)
					us_cf_cycleStep5=0;
			}
			break;
		}
		case 8:
		{ 	// 4s 执行一次
			if(us_cf_cycleStep2<=10)
			{
				st_cf_cycleTxId.ul_funId = 0x100;
				st_cf_cycleTxId.us_doorType=FDOOR;//前门按钮灯
				if(us_cf_cycleStep2==0)
					st_cf_cycleTxId.us_subId = 0x60;
				else if(us_cf_cycleStep2==1)
					st_cf_cycleTxId.us_subId = 0x61;
				else if(us_cf_cycleStep2==2)
					st_cf_cycleTxId.us_subId = 0x62;
				else if(us_cf_cycleStep2==3)
					st_cf_cycleTxId.us_subId = 0x63;
				else if(us_cf_cycleStep2==4)
					st_cf_cycleTxId.us_subId = 0x64;
				else if(us_cf_cycleStep2==5)
					st_cf_cycleTxId.us_subId = 0x65; //A01.7-CR001
                else if(us_cf_cycleStep2==6)
					st_cf_cycleTxId.us_subId = 0x66;
                else if(us_cf_cycleStep2==7)
					st_cf_cycleTxId.us_subId = 0x67;
				else if(us_cf_cycleStep2==8)
					st_cf_cycleTxId.us_subId = 0x68; //A02.0-CR016  v1.1
				else if(us_cf_cycleStep2==9)
				{
					st_cf_cycleTxId.us_subId = 0x80;
					st_cf_cycleTxId.us_doorType=FBDOOR;
				}
				else if(us_cf_cycleStep2==10)
					st_cf_cycleTxId.us_subId = 0x7E; //A02.2-CR038v1.0
			}
			else if((us_cf_cycleStep2>10)&&(us_cf_cycleStep2<19))//所有计数都必须被使用，否则0x100,0x00是读轿顶板EID//A01.7-CR001
			{
				st_cf_cycleTxId.ul_funId = 0x100;
				st_cf_cycleTxId.us_doorType=BDOOR;//后门按钮灯
				if(us_cf_cycleStep2==11)
					st_cf_cycleTxId.us_subId = 0x60;
				else if(us_cf_cycleStep2==12)
					st_cf_cycleTxId.us_subId = 0x61;
				else if(us_cf_cycleStep2==13)
					st_cf_cycleTxId.us_subId = 0x62;
				else if(us_cf_cycleStep2==14)
					st_cf_cycleTxId.us_subId = 0x63;
				else if(us_cf_cycleStep2==15)
					st_cf_cycleTxId.us_subId = 0x64;
                else if(us_cf_cycleStep2==16)
					st_cf_cycleTxId.us_subId = 0x65;
				else if(us_cf_cycleStep2==17)
					st_cf_cycleTxId.us_subId = 0x66;
 				else if(us_cf_cycleStep2==18)
					st_cf_cycleTxId.us_subId = 0x67;               
			}	
			us_cf_cycleStep2++;
			if(us_cf_cycleStep2>19)
				us_cf_cycleStep2=0;
			break;
		}
		case 9:
		{	
			st_cf_cycleTxId.ul_funId = 0x122;
			if(us_cf_cycleStep3<=8)//A01.7-CR001
			{
				st_cf_cycleTxId.us_doorType=FDOOR;//前门残障按钮灯
				if(us_cf_cycleStep3==0)
					st_cf_cycleTxId.us_subId = 0x60;
				else if(us_cf_cycleStep3==1)
					st_cf_cycleTxId.us_subId = 0x61;
				else if(us_cf_cycleStep3==2)
					st_cf_cycleTxId.us_subId = 0x62;
				else if(us_cf_cycleStep3==3)
					st_cf_cycleTxId.us_subId = 0x63;
				else if(us_cf_cycleStep3==4)
					st_cf_cycleTxId.us_subId = 0x64;
				else if(us_cf_cycleStep3==5)
					st_cf_cycleTxId.us_subId = 0x65;
				else if(us_cf_cycleStep3==6)
					st_cf_cycleTxId.us_subId = 0x66; 
                else if(us_cf_cycleStep3==7)
					st_cf_cycleTxId.us_subId = 0x67; 
				else if(us_cf_cycleStep3==8)
				{
					st_cf_cycleTxId.us_subId = 0x80;
					st_cf_cycleTxId.us_doorType=FBDOOR;
				}
			}
			else if((us_cf_cycleStep3>8)&&(us_cf_cycleStep3<17))//A01.7-CR001
			{
				st_cf_cycleTxId.us_doorType=BDOOR;//后门残障按钮灯
				if(us_cf_cycleStep3==9)
					st_cf_cycleTxId.us_subId = 0x60;
				else if(us_cf_cycleStep3==10)
					st_cf_cycleTxId.us_subId = 0x61;
				else if(us_cf_cycleStep3==11)
					st_cf_cycleTxId.us_subId = 0x62;
				else if(us_cf_cycleStep3==12)
					st_cf_cycleTxId.us_subId = 0x63;
				else if(us_cf_cycleStep3==13)
					st_cf_cycleTxId.us_subId = 0x64;
                else if(us_cf_cycleStep3==14)
					st_cf_cycleTxId.us_subId = 0x65;
				else if(us_cf_cycleStep3==15)
					st_cf_cycleTxId.us_subId = 0x66;
                else if(us_cf_cycleStep3==16)
					st_cf_cycleTxId.us_subId = 0x67;
			}
			us_cf_cycleStep3++;
			if(us_cf_cycleStep3>19)
				us_cf_cycleStep3=0;
			break;		
		}
		default:
			break;
	}
	us_cf_cycleStep++;
	if(us_cf_cycleStep>9)
		us_cf_cycleStep=0;
}

/***********************************************************************
**函数功能:	判断所有事件的紧急程度
***********************************************************************/
void Cf_EvtWaitTimeInc(void)
{
	unsigned short i;
	unsigned short us_waitPercentTmp = 0;

	us_cf_insertWaitLongest = 0xffff;
	
	for(i=0; i<CF_INSERT_MAX1; i++ )
	{
		if(st_cf_insertReq[i].ul_funId)
		{
			st_cf_insertReq[i].us_timeCur += CF_10MS;
			if(st_cf_insertReq[i].us_timeCur > st_cf_insertReq[i].us_timeLimit)
			{
				st_cf_insertReq[i].us_timeCur = st_cf_insertReq[i].us_timeLimit; 
			}
			if(st_cf_insertReq[i].us_timeLimit==0)
			{
				st_cf_insertReq[i].us_waitPercent = 100;
			}
			else
			{
				st_cf_insertReq[i].us_waitPercent = (unsigned long)(st_cf_insertReq[i].us_timeCur*100)/st_cf_insertReq[i].us_timeLimit;
			}
			if(st_cf_insertReq[i].us_waitPercent > us_waitPercentTmp)
			{
				us_waitPercentTmp = st_cf_insertReq[i].us_waitPercent;
				us_cf_insertWaitLongest = i;
			}
		}
	}
    
    if(FRM_UART_TO_CAN1 == us_cf_insertWaitLongest)
    {
        us_cf_insertWaitLongest = FRM_UART_TO_CAN1;
    }
    
}
void Cf_TxIdJudge(void)
{
	st_cf_sendFrame.COB_ID = 0;
	st_cf_sendFrame.ul_funId = 0;
	st_cf_sendFrame.us_subId = 0;
	st_cf_sendFrame.us_doorType = 0;
  	st_cf_sendFrame.uc_exId = 0;    //V20.12 初始化默认标准帧

	us_cf_judgeToggle ^= 0x01;
	if(us_cf_judgeToggle&0x01)
	{
		if(us_cf_insertWaitLongest != 0xffff)//有紧急的事件
		{
			st_cf_sendFrame.ul_funId = st_cf_insertReq[us_cf_insertWaitLongest].ul_funId;
			st_cf_sendFrame.us_subId = st_cf_insertReq[us_cf_insertWaitLongest].us_subId;
			st_cf_sendFrame.uc_exId = st_cf_insertReq[us_cf_insertWaitLongest].uc_exId;
			st_cf_sendFrame.us_doorType = st_cf_insertReq[us_cf_insertWaitLongest].us_doorType;
			st_cf_insertReq[us_cf_insertWaitLongest].us_tickLeft--;
			st_cf_insertReq[us_cf_insertWaitLongest].us_timeCur = 0;
			if(st_cf_insertReq[us_cf_insertWaitLongest].us_tickLeft==0)
			{
				st_cf_insertReq[us_cf_insertWaitLongest].ul_funId = 0;
				st_cf_insertReq[us_cf_insertWaitLongest].us_subId = 0;
				st_cf_insertReq[us_cf_insertWaitLongest].uc_exId = 0;
				st_cf_insertReq[us_cf_insertWaitLongest].us_timeLimit = 0;
				st_cf_insertReq[us_cf_insertWaitLongest].us_waitPercent = 0;
				st_cf_insertReq[us_cf_insertWaitLongest].us_doorType = 0;
			}
			Cf_MaskEvtFrameChk(st_cf_sendFrame.ul_funId,st_cf_sendFrame.us_subId);
		}
		else if(st_cf_cycleTxId.ul_funId)
		{
			st_cf_sendFrame.ul_funId = st_cf_cycleTxId.ul_funId;
			st_cf_sendFrame.us_subId = st_cf_cycleTxId.us_subId;
			if((st_cf_sendFrame.ul_funId==0x1fff)||(st_cf_sendFrame.ul_funId==0x5fff))   //A02.3-CR028
				st_cf_sendFrame.uc_exId = 1;
			else
				st_cf_sendFrame.uc_exId = 0;    //其余均为标准帧
			st_cf_sendFrame.us_doorType = st_cf_cycleTxId.us_doorType;
			st_cf_cycleTxId.ul_funId =0;
			st_cf_cycleTxId.us_subId =0;
			st_cf_cycleTxId.us_doorType =0;
		}
	}
	else
	{
		if(st_cf_cycleTxId.ul_funId)
		{
			st_cf_sendFrame.ul_funId = st_cf_cycleTxId.ul_funId;
			st_cf_sendFrame.us_subId = st_cf_cycleTxId.us_subId;
			if((st_cf_sendFrame.ul_funId==0x1fff)||(st_cf_sendFrame.ul_funId==0x5fff))    //A02.3-CR028
				st_cf_sendFrame.uc_exId = 1;
			else
				st_cf_sendFrame.uc_exId = 0;    //其余均为标准帧
			st_cf_sendFrame.us_doorType = st_cf_cycleTxId.us_doorType;
			st_cf_cycleTxId.ul_funId =0;
			st_cf_cycleTxId.us_subId =0;
			st_cf_cycleTxId.us_doorType =0;
		}
	}
}

/*************************************************************
*@brief【描述】
*@author mdq
*@date   2023-11-09
*@note 【备注】
*************************************************************/
void Cf_CanTxData(void)
{
	unsigned short i;
	
	st_cf_sendFrame.COB_ID = st_cf_sendFrame.ul_funId;
	for(i=0; i<8; i++)
		st_cf_sendFrame.Data[i] = 0;

	if(st_cf_sendFrame.COB_ID==0x100)
	{
		switch(st_cf_sendFrame.us_subId)
		{
			case 0x60:
			{
				if(st_cf_sendFrame.us_doorType==FDOOR)
					Cf_PackFrm100Ide60Fdoor(st_cf_sendFrame.Data);
				else if(st_cf_sendFrame.us_doorType==BDOOR)
					Cf_PackFrm100Ide60Bdoor(st_cf_sendFrame.Data);
				else
					st_cf_sendFrame.COB_ID=0;
				st_cf_sendFrame.DLC = 8;
				break;			
			}
			case 0x61:
			{
				if(st_cf_sendFrame.us_doorType==FDOOR)
					Cf_PackFrm100Ide61Fdoor(st_cf_sendFrame.Data);
				else if(st_cf_sendFrame.us_doorType==BDOOR)
					Cf_PackFrm100Ide61Bdoor(st_cf_sendFrame.Data);
				else
					st_cf_sendFrame.COB_ID=0;
				st_cf_sendFrame.DLC = 8;
				break;			
			}
			default:
			{
				st_cf_sendFrame.COB_ID=0;
				break;
			}
		}
	}      
	else if(st_cf_sendFrame.COB_ID==0x417)
	{
		if(st_cf_sendFrame.us_subId==0xE4)
		{
			Cf_PackFrm417IdeE4(st_cf_sendFrame.Data);
			st_cf_sendFrame.DLC = 8;
		}
	}
	if(st_cf_sendFrame.COB_ID)
	{
		us_cf_needSend = 1;
	}
}
/*************************************************************
*@brief【描述】
*@param  puc_data    【参数注释】
*@author mdq
*@date   2023-11-10
*@note 【备注】
*************************************************************/
void Cf_PackFrm100Ide60Fdoor(unsigned char * puc_data) //前门主副1~8层灯
{
	static unsigned char uc_dataBak = 0;
	
	puc_data[0] = ((LIFT_NUM_CAN&0xff)<<4)+0x05;
	puc_data[1] = 0;
	puc_data[2] = 0x60; 
	puc_data[3] = 0xFF;
	puc_data[4] = cfDoor[0].cop_out_lamp[0];
	puc_data[5] = 0x01;
	puc_data[6] = 0;
	puc_data[7] = 0;
	
	Cf_TxMaskDeal(&uc_dataBak,&puc_data[3],&puc_data[4]);
}

void Cf_InsertReqList(unsigned short us_index,unsigned long ul_funId,unsigned short us_subId,unsigned char uc_exId,unsigned short us_tickTotal,unsigned short us_timeLimit,unsigned short us_doorType)
{
	if((st_cf_insertReq[us_index].ul_funId == ul_funId) && (st_cf_insertReq[us_index].us_subId == us_subId)
	     &&(st_cf_insertReq[us_index].us_doorType==us_doorType))
	{
		st_cf_insertReq[us_index].us_tickLeft = us_tickTotal;
	}
	else
	{
		st_cf_insertReq[us_index].ul_funId = ul_funId;
		st_cf_insertReq[us_index].us_subId = us_subId;
    		st_cf_insertReq[us_index].uc_exId = uc_exId;
		st_cf_insertReq[us_index].us_tickLeft = us_tickTotal;
		st_cf_insertReq[us_index].us_timeCur = 0;
		st_cf_insertReq[us_index].us_timeLimit = us_timeLimit;
		st_cf_insertReq[us_index].us_waitPercent = 0;
		st_cf_insertReq[us_index].us_doorType =	us_doorType;

	}
}