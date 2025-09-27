#pragma once

#ifdef __cplusplus
extern "C" {
#endif


	//链接lib
#ifndef _NOLINKSP

#if _WIN64
#if _MT
#if _DEBUG
#if _DLL
#pragma comment(lib,"SPCloud64_MDd.lib")//MDd
#else
#pragma comment(lib,"SPCloud64_MTd.lib")//MTd
#endif // _DLL
#else
#if _DLL
#pragma comment(lib,"SPCloud64_MD.lib")//MD
#else
#pragma comment(lib,"SPCloud64_MT.lib")//MT
#endif // _DLL
#endif // _DEBUG
#endif // _MT
#else
#if _MT
#if _DEBUG
#if _DLL
#pragma comment(lib,"SPCloud_MDd.lib")//MDd
#else
#pragma comment(lib,"SPCloud_MTd.lib")//MTd
#endif // _DLL
#else
#if _DLL
#pragma comment(lib,"SPCloud_MD.lib")//MD
#else
#pragma comment(lib,"SPCloud_MT.lib")//MT
#endif // _DLL
#endif // _DEBUG
#endif // _MT
#endif // _WIN64
#endif // !_NOLINKSP

#ifdef _USERDLL
#define DLLEX __declspec(dllexport)
#else
#define DLLEX 
#endif // _DLL

#ifndef IN
#define IN
#endif

#ifndef OUT
#define OUT
#endif

#ifndef OPTIONAL
#define OPTIONAL
#endif

#ifdef _WIN64
#define SPAPI
#else
#define SPAPI __stdcall
#endif // _WIN64

    /// 下面是接口


	/* 描述: 机器码绑定信息 */
#pragma pack(push)
#pragma pack(1)
	struct tagPCSignInfo {
		// 绑定时间戳
		unsigned long long	u64BindTS;

		// 操作系统版本
		char*				szWinVer;

		// 备注(Clientmark.txt内容)
		char*				szRemark;

		// 计算机名
		char*				szComputerName;

		// 机器码
		char*				szPCSign;

		// 最后登录时间
		unsigned long long	u64LastLoginTS;

		// 保留字段;
		void*				Reserved[20];
	};
#pragma pack(pop)

	/* 描述: 机器码绑定信息 */
#pragma pack(push)
#pragma pack(1)
	struct tagPCSignInfoHead {
		unsigned int		u32Count;
		tagPCSignInfo*	Info;
		unsigned int		u32BindIP;//		是否绑定IP(0/1);	
		unsigned int		u32RestCount;// 周期内剩余次数
		unsigned long long		u64RefreshCountdownSeconds;// 秒数; 用于转换成距离[换绑次数更新时间]还剩：xxx天xx时xx分
		unsigned int		u32Limit;// 限制解绑; 0:无限制, 1仅允许已绑定的机器码进行解绑
		void*			Reserved[19];
	};
#pragma pack(pop)

	/* 描述: 客户端在线信息 */
#pragma pack(push)
#pragma pack(1)
	struct tagOnlineInfo {
		unsigned int	u32CID;
		char*		szComputerName;
		char*		szWinVer;
		unsigned long long	u64CloudInitTS;
		void*		Reserved[20];
	};
#pragma pack(pop)

	/* 描述: 客户端在线信息参数 */
#pragma pack(push)
#pragma pack(1)
	struct tagOnlineInfoHead {
		unsigned int			u32Count;
		tagOnlineInfo*		Info;
		void*				Reserved[20];
	};
#pragma pack(pop)

	/* 描述: 用户充值成功信息 */
#pragma pack(push)
#pragma pack(1)
	struct tagUserRechargedInfo {
		unsigned long long	u64OldExpiredTimeStamp;//	旧的卡密过期时间戳;
		unsigned long long	u64NewExpiredTimeStamp;//	新的卡密过期时间戳;
		unsigned long long	u64OldFYI;//				旧的点数;
		unsigned long long	u64NewFYI;//				新的点数;
		unsigned int	u32RechargeCount;//			本次充值的卡密个数;
		void* Reserved[80];//					保留字段;
	};
#pragma pack(pop)

#pragma pack(push)
#pragma pack(1)
	struct tagBasicInfo {
		unsigned int	ForbidTrial;		// 1禁止, 0允许		禁止试用
		unsigned int	ForbidLogin;		// 1禁止, 0允许		禁止软件登录
		unsigned int	ForbidRegister;	// 1禁止, 0允许		禁止软件注册
		unsigned int	ForbidRecharge;	// 1禁止, 0允许		禁止软件充值
		unsigned int	ForbidCloudGetCountinfo;//	1/0		禁止客户端云计算使用 [获取频率验证总在线数]和[获取在线卡密数]
		unsigned int	Reserved[15];//			保留字段;
	};
#pragma pack(pop)

	/* 描述: 云计算, 创建一个云计算对象 */
	/* 返回: 成功返回对象指针(云计算所有函数都会用到), 失败返回0  */
	DLLEX void* SPAPI SP_Cloud_Create();

	/* 描述: 云计算, 销毁一个云计算对象 */
	/*       本函数内部有调用SP_Cloud_Offline做离线处理, 但是可能会因为时机问题导致无法离线 */
	/*       简易最好是调用SP_Cloud_Destroy之前先调用SP_Cloud_Offline下线 */
	/* 参数: pContext; 云计算对象; */
	DLLEX void SPAPI SP_Cloud_Destroy(void* pContext);

	/* 描述: 云计算初始化;(python/VMP无法使用) */
	/* 参数: pContext; 云计算对象; */
	/* 参数: iTimeout; send/recv超时时间(毫秒), 建议30*1000 */
	/* 参数: iError; 云计算错误码 */
	/* 返回: 是否初始化成功; 如果出错, 可参考iError  */
	DLLEX bool  SPAPI SP_CloudInit(void* pContext, int iTimeout, OPTIONAL int* iError);

	/* 描述: SP云计算_设置连接信息; 不可与SP_CloudInit函数一起使用! */
	/* 参数: pContext; 云对象; */
	/* 参数: szSoftwareName; 软件名; */
	/* 参数: szIP; IP/域名; */
	/* 参数: wPort; 端口; */
	/* 参数: iTimeout; 超时时间; */
	/* 参数: iLocalVer; 本地版本号; (暂未实现)*/
	/* 参数: bPopMsg; 是否弹窗提示; */
	DLLEX void SPAPI SP_CloudSetConnInfo(void* pContext, const char* szSoftwareName, const char* szIP, int wPort, int iTimeout, int iLocalVer, bool bPopMsg);

	/* 描述: SP云计算_登录; 不可与SP_CloudInit函数一起使用! */
	/*		需要先调用SP_CloudSetConnInfo配置连接参数; */
	/*		本函数相当于 源码接入版的"登录函数", 登陆成功后, 可以照常使用所有云计算函数; 可以配合使用SP或VMP加壳; */
	/* 参数: pContext; 云计算对象; */
	/* 参数: szCard; 卡密; */
	/* 参数: szUser; 账号; */
	/* 参数: szPassword; 密码; */
	/* 参数: iError; 云计算错误码 */
	/* 返回: 是否登录成功; 如果出错, 可参考iError  */
	DLLEX bool  SPAPI SP_CloudLogin(void* pContext, const char* szCard, OPTIONAL int* iError);
	DLLEX bool  SPAPI SP_CloudUserLogin(void* pContext, const char* szUser, const char* szPassword, OPTIONAL int* iError);

	/* 描述: 云计算请求 (每次调用联网) */
	/*		该函数返回true时, pOutBuffer若不为0, 则需要用户自己释放内存 SP_Cloud_Free(pOutBuffer) */
	/* 参数: pContext; 云计算对象; */
	/* 参数: dwCloudID; 云计算ID; (必须大于0) */
	/* 参数: pInBuffer; 云计算数据包指针 */
	/* 参数: dwInLength; 云计算数据包长度 */
	/* 参数: pOutBuffer; 返回的数据包数据 */
	/* 参数: dwOutLength; 返回的数据包长度 */
	/* 参数: iError; 云计算错误码 */
	/* 参数: u32RetryCount; 重试次数; 如果通讯过程中出现网络错误, 则会通过这个参数决定重试次数; */
	/* 参数: u32RetryIntervalMs; 重试间隔; 如果u32RetryCount不为0, 则每次重发间隔时长由此值决定(毫秒); */
	/* 返回: 云计算是否成功; 如果出错, 可参考iError */
	DLLEX bool  SPAPI SP_CloudComputing(void* pContext, IN int dwCloudID, IN OPTIONAL unsigned char* pInBuffer, IN OPTIONAL unsigned int dwInLength, OUT OPTIONAL unsigned char** pOutBuffer, OUT OPTIONAL unsigned int* dwOutLength, OPTIONAL int* iError, 
		unsigned int u32RetryCount = 0, unsigned int u32RetryIntervalMs = 0);

	/* 描述: 云计算, 频率验证 (每次调用联网; 建议创建一条线程来频繁调用, 比如30秒调用一次) */
	/* 参数: pContext; 云计算对象; */
	/* 参数: iError; 云计算错误码 */
	/* 返回: 是否验证成功; 如果出错, 可参考iError  */
	DLLEX bool  SPAPI SP_Cloud_Beat(void* pContext, OPTIONAL int* iError);

	/* 描述: 云计算, 获取当前登陆卡密所属代理名 (每次调用联网) */
	/* 参数: pContext; 云计算对象; */
	/* 参数: szAgent[44] */
	/* 参数: iError; 云计算错误码 */
	/* 返回: 是否操作成功; 如果出错, 可参考iError  */
	DLLEX bool  SPAPI SP_Cloud_GetCardAgent(void* pContext, char szAgent[44], OPTIONAL int* iError);

	/* 描述: 云计算, 获取当前登陆卡密的卡类型 (每次调用联网) */
	/* 参数: pContext; 云计算对象; */
	/* 参数: szCardType[36] */
	/* 参数: iError; 云计算错误码 */
	/* 返回: 是否操作成功; 如果出错, 可参考iError  */
	DLLEX bool  SPAPI SP_Cloud_GetCardType(void* pContext, char szCardType[36], OPTIONAL int* iError);

	/* 描述: 云计算, 获取当前登陆卡密登录时记录的IP地址 (每次调用联网) */
	/* 参数: pContext; 云计算对象; */
	/* 参数: szIPAddress[44] */
	/* 参数: iError; 云计算错误码 */
	/* 返回: 是否操作成功; 如果出错, 可参考iError  */
	DLLEX bool  SPAPI SP_Cloud_GetIPAddress(void* pContext, char szIPAddress[44], OPTIONAL int* iError);

	/* 描述: 云计算, 获取当前登陆卡密的备注 (每次调用联网) */
	/* 参数: pContext; 云计算对象; */
	/* 参数: szRemarks[132] */
	/* 参数: iError; 云计算错误码 */
	/* 返回: 是否操作成功; 如果出错, 可参考iError  */
	DLLEX bool  SPAPI SP_Cloud_GetRemarks(void* pContext, char szRemarks[132], OPTIONAL int* iError);

	/* 描述: 云计算, 获取当前登陆卡密的创建时间戳 (每次调用联网) */
	/* 参数: pContext; 云计算对象; */
	/* 参数: iCreatedTimeStamp */
	/* 参数: iError; 云计算错误码 */
	/* 返回: 是否操作成功; 如果出错, 可参考iError  */
	DLLEX bool  SPAPI SP_Cloud_GetCreatedTimeStamp(void* pContext, __int64* iCreatedTimeStamp, OPTIONAL int* iError);

	/* 描述: 云计算, 获取当前登陆卡密的激活时间戳 (每次调用联网) */
	/* 参数: pContext; 云计算对象; */
	/* 参数: iActivatedTimeStamp */
	/* 参数: iError; 云计算错误码 */
	/* 返回: 是否操作成功; 如果出错, 可参考iError  */
	DLLEX bool  SPAPI SP_Cloud_GetActivatedTimeStamp(void* pContext, __int64* iActivatedTimeStamp, OPTIONAL int* iError);

	/* 描述: 云计算, 获取当前登陆卡密的过期时间戳 (每次调用联网) */
	/* 参数: pContext; 云计算对象; */
	/* 参数: iExpiredTimeStamp */
	/* 参数: iError; 云计算错误码 */
	/* 返回: 是否操作成功; 如果出错, 可参考iError  */
	DLLEX bool  SPAPI SP_Cloud_GetExpiredTimeStamp(void* pContext, __int64* iExpiredTimeStamp, OPTIONAL int* iError);

	/* 描述: 云计算, 获取当前登陆卡密的最后登录时间戳 (每次调用联网) */
	/* 参数: pContext; 云计算对象; */
	/* 参数: iLastLoginTimeStamp */
	/* 参数: iError; 云计算错误码 */
	/* 返回: 是否操作成功; 如果出错, 可参考iError  */
	DLLEX bool  SPAPI SP_Cloud_GetLastLoginTimeStamp(void* pContext, __int64* iLastLoginTimeStamp, OPTIONAL int* iError);

	/* 描述: 云计算, 获取当前登陆卡密的剩余点数 (每次调用联网) */
	/* 参数: pContext; 云计算对象; */
	/* 参数: iFYI */
	/* 参数: iError; 云计算错误码 */
	/* 返回: 是否操作成功; 如果出错, 可参考iError  */
	DLLEX bool  SPAPI SP_Cloud_GetFYI(void* pContext, __int64* iFYI, OPTIONAL int* iError);

	/* 描述: 扣除当前卡密点数; 用于用户使用了某些特殊功能需要额外扣费的场景 (每次调用联网) */
	/* 参数: pContext; 云计算对象; */
	/* 参数：iFYICount：需要扣除的点数数量*/
	/* 参数：iSurplusFYIOUT 剩下的点数; 返回值为FALSE时, -2欲扣除点数为0或负数, -1剩余点数不足导致扣除失败; */
	/* 参数: iError; 云计算错误码 */
	/* 返回: 是否操作成功; 如果出错, 可参考iError  */
	DLLEX bool  SPAPI SP_Cloud_DeductFYI(void* pContext, __int64 iFYICount, __int64* iSurplusFYI, OPTIONAL int* iError);

	/* 描述: 云计算, 获取当前登陆卡密的多开数量属性值 (每次调用联网) */
	/* 参数: pContext; 云计算对象; */
	/* 参数: iNum */
	/* 参数: iError; 云计算错误码 */
	/* 返回: 是否操作成功; 如果出错, 可参考iError  */
	DLLEX bool  SPAPI SP_Cloud_GetOpenMaxNum(void* pContext, int* iNum, OPTIONAL int* iError);

	/* 描述: 云计算, 获取当前登陆卡密的绑定机器属性值 (每次调用联网) */
	/* 参数: pContext; 云计算对象; */
	/* 参数: iBind; 是否绑机, 1/0 */
	/* 参数: iError; 云计算错误码 */
	/* 返回: 是否操作成功; 如果出错, 可参考iError  */
	DLLEX bool  SPAPI SP_Cloud_GetBind(void* pContext, int* iBind, OPTIONAL int* iError);

	/* 描述: 云计算, 获取当前登陆卡密的换绑周期 (每次调用联网) */
	/* 参数: pContext; 云计算对象; */
	/* 参数: iBindTime; (秒) */
	/* 参数: iError; 云计算错误码 */
	/* 返回: 是否操作成功; 如果出错, 可参考iError  */
	DLLEX bool  SPAPI SP_Cloud_GetBindTime(void* pContext, __int64* iBindTime, OPTIONAL int* iError);

	/* 描述: 云计算, 获取当前登陆卡密的解绑扣除属性值 (每次调用联网) */
	/* 参数: pContext; 云计算对象; */
	/* 参数: iDeductSec; (秒) */
	/* 参数: iError; 云计算错误码 */
	/* 返回: 是否操作成功; 如果出错, 可参考iError  */
	DLLEX bool  SPAPI SP_Cloud_GetUnBindDeductTime(void* pContext, __int64* iDeductSec, OPTIONAL int* iError);

	/* 描述: 云计算, 获取当前登陆卡密的最多解绑次数属性值 (每次调用联网) */
	/* 参数: pContext; 云计算对象; */
	/* 参数: iNum */
	/* 参数: iError; 云计算错误码 */
	/* 返回: 是否操作成功; 如果出错, 可参考iError  */
	DLLEX bool  SPAPI SP_Cloud_GetUnBindMaxNum(void* pContext, int* iNum, OPTIONAL int* iError);

	/* 描述: 云计算, 获取当前登陆卡密的累计解绑次数 (每次调用联网) */
	/* 参数: pContext; 云计算对象; */
	/* 参数: iCountTotal */
	/* 参数: iError; 云计算错误码 */
	/* 返回: 是否操作成功; 如果出错, 可参考iError  */
	DLLEX bool  SPAPI SP_Cloud_GetUnBindCountTotal(void* pContext, int* iCountTotal, OPTIONAL int* iError);

	/* 描述: 云计算, 获取当前登陆卡密的累计解绑扣除的时间 (每次调用联网) */
	/* 参数: pContext; 云计算对象; */
	/* 参数: iDeductTimeTotal; (秒) */
	/* 参数: iError; 云计算错误码 */
	/* 返回: 是否操作成功; 如果出错, 可参考iError  */
	DLLEX bool  SPAPI SP_Cloud_GetUnBindDeductTimeTotal(void* pContext, __int64* iDeductTimeTotal, OPTIONAL int* iError);

	/* 描述: 云计算, 移除当前云计算身份认证信息 (每次调用联网) */
	/* 参数: pContext; 云计算对象; */
	/* 参数: iError; 云计算错误码 */
	/* 返回: 是否操作成功; 如果出错, 可参考iError  */
	DLLEX bool  SPAPI SP_Cloud_Offline(void* pContext, OPTIONAL int* iError);

	/* 描述: 通用; 获取公告内容 (每次调用联网) */
	/* 参数: pContext; 云计算对象; */
	/* 参数: szNoteices */
	/* 返回: 是否操作成功; 如果出错, 可参考iError  */
	DLLEX bool  SPAPI SP_Cloud_GetNotices(void* pContext, char szNoteices[65535], OPTIONAL int* iError);

	/* 描述: 云计算, 获取当前登陆的卡密 (不联网; SP_CloudInit 初始化成功后可用) */
	/* 参数: pContext; 云计算对象; */
	/* 参数: szCard */
	/* 返回: 是否操作成功; 如果出错, 可参考iError  */
	DLLEX bool  SPAPI SP_Cloud_GetCard(void* pContext, char szCard[1 + 41], OPTIONAL int* iError);

	/* 描述: 云计算, 获取当前登陆的账号 (不联网; SP_CloudInit 初始化成功后可用) */
	/* 参数: pContext; 云计算对象; */
	/* 参数: szUser */
	/* 返回: 是否操作成功; 如果出错, 可参考iError  */
	DLLEX bool  SPAPI SP_Cloud_GetUser(void* pContext, char szCard[1 + 32], OPTIONAL int* iError);

	/* 描述: 云计算, 禁用当前登陆的卡密 (每次调用联网; SP_CloudInit 初始化成功后可用) */
	/* 参数: pContext; 云计算对象; */
	/* 参数: iError; 云计算错误码 */
	/* 返回: 无; 如果出错, 可参考iError  */
	DLLEX void  SPAPI SP_Cloud_DisableCard(void* pContext, OPTIONAL int* iError);

	/* 描述: 云计算, 获取当前客户端ID (不联网; SP_CloudInit 初始化成功后可用) */
	/* 参数: pContext; 云计算对象; */
	/* 返回: 是否操作成功; 如果出错, 可参考iError  */
	DLLEX bool  SPAPI SP_Cloud_GetCID(void* pContext, OUT int* pCID, OPTIONAL int* iError);

	/* 描述: 云计算, 获取当前卡密在线客户端数量 (SP_CloudInit 初始化成功后可用; 每次调用联网) */
	/* 参数: pContext; 云计算对象; */
	/* 参数: iError; 云计算错误码 */
	/* 返回: 是否操作成功; 如果出错, 可参考iError  */
	DLLEX bool  SPAPI SP_Cloud_GetOnlineCount(void* pContext, int* iCount, OPTIONAL int* iError);

	/* 描述: 云计算, 设置云计算操作系统版本标识 (不联网; SP_CloudInit 初始化之前使用) */
	/* 参数: pContext; 云计算对象; */
	/* 参数: szWinVer; 自定义操作系统版本标识, 如果为空, 则为内置逻辑获取操作系统版本 */
	/* 返回: 是否操作成功; 如果出错, 可参考iError  */
	DLLEX bool  SPAPI SP_Cloud_SetWinVer(void* pContext, const char* szWinVer, OPTIONAL int* iError);

	/* 描述: 云计算, 获取网络验证登录时使用的机器码 (不联网); 注意!!! 本接口仅在使用SP_CloudInit且编译生成的软件经过SP加密后生效!!! */
	/* 参数: pContext; 云计算对象; */
	/* 参数: szPCSign[33] */
	/* 返回: 是否操作成功; 如果出错, szPCSign数据无意义;  */
	DLLEX bool  SPAPI SP_Cloud_GetPCSign(void* pContext, char szPCSign[33]);

	/* 描述: 云计算, 获取当前登陆卡密周期内的解绑次数 (每次调用联网) */
	/* 参数: pContext; 云计算对象; */
	/* 参数: iCount */
	/* 参数: iError; 云计算错误码 */
	/* 返回: 是否操作成功; 如果出错, 可参考iError  */
	DLLEX bool  SPAPI SP_Cloud_GetUnBindCount(void* pContext, int* iCount, OPTIONAL int* iError);

	/* 描述: 云计算, 获取服务端版本配置信息 (每次调用联网) */
	/* 参数: pContext; 云计算对象; */
	/* 参数: int* bForce;		可选; 是否强制更新(1强制, 0不强制) */
	/* 参数: int* dwVer;			可选; 版本号 */
	/* 参数: int* bDirectUrl;	可选; 是否为直链(1直连, 0非直链) */
	/* 参数: char szUrl[2049];	可选; 下载地址 */
	/* 参数: char szRunExe[101];	可选; 下载后运行的exe名 */
	/* 参数: char szRunCmd[129];	可选; 下载后运行exe的参数 */
	/* 参数: int* iError;		可选; 云计算错误码 */
	/* 返回: 是否操作成功; 如果出错, 可参考iError  */
	DLLEX bool  SPAPI SP_Cloud_GetUpdateInfo(void* pContext,
		OUT OPTIONAL int* bForce,/*			是否强制更新(BOOL) */
		OUT OPTIONAL int* dwVer,/*			版本号 */
		OUT OPTIONAL int* bDirectUrl,/*		是否为直链(BOOL) */
		OUT OPTIONAL char szUrl[2049],/*		下载地址 */
		OUT OPTIONAL char szRunExe[101],/*	下载后运行的exe名 */
		OUT OPTIONAL char szRunCmd[129],/*	下载后运行exe的参数 */
		OPTIONAL int* iError);

	/* 描述: 云计算, 获取本地版本号 (不联网; 加密后, SP_CloudInit 初始化成功后可用) */
	/* 参数: pContext; 云计算对象; */
	/* 返回: [加密端->网络验证配置]页面输入的版本号/设置连接信息参数里的版本号 */
	DLLEX int   SPAPI SP_Cloud_GetLocalVerNumber(void* pContext);

	/* 描述: 云计算, 获取频率验证总在线数量 (每次调用联网; 该功能需要在服务端 [独立软件管理] 开启) */
	/* 参数: pContext; 云计算对象; */
	/* 参数: iTotalCount; 总在线数量 */
	/* 参数: iError; 云计算错误码 */
	/* 返回: 是否操作成功; 如果出错, 可参考iError  */
	DLLEX bool  SPAPI SP_Cloud_GetOnlineTotalCount(void* pContext, unsigned int* iTotalCount, OPTIONAL int* iError);

	/* 描述: 云计算, 获取在线卡密数量 (每次调用联网; 该功能需要在服务端 [独立软件管理] 开启) */
	/* 参数: pContext; 云计算对象; */
	/* 参数: iTotalCount; 总在线数量 */
	/* 参数: iError; 云计算错误码 */
	/* 返回: 是否操作成功; 如果出错, 可参考iError  */
	/* 说明: */
	/*		比如有100张多开数量为10的卡密, 所有卡密都已登录占满多开数量 */
	/*		此时服务端拥有100*10=1000条在线链接 */
	/*		1000条在线链接中实际上是有100张在线卡密, 调用当前接口后, iTotalCount值则为100 */
	DLLEX bool  SPAPI SP_Cloud_GetOnlineCardsCount(void* pContext, unsigned int* iTotalCount, OPTIONAL int* iError);

	/* 描述: 云计算, 获取指定卡密在线链接数量 (每次调用联网) */
	/* 参数: pContext; 云计算对象; */
	/* 参数: szCard; 卡密; 填写NULL为当前登录云计算的卡密 */
	/* 参数: iTotalCount; 总在线数量 */
	/* 参数: iError; 云计算错误码 */
	/* 返回: 是否操作成功; 如果出错, 可参考iError  */
	/* 说明: */
	/*		比如有1张多开数量为10的卡密 */
	/*		此时用户使用这张卡密登录了3个客户端, 调用当前接口后, iTotalCount值则为3 */
	DLLEX bool  SPAPI SP_Cloud_GetOnlineCountByCard(void* pContext, OPTIONAL const char* szCard, unsigned int* iTotalCount, OPTIONAL int* iError);

	/* 描述: 云计算, 获取指定卡密机器码绑定信息 (每次调用联网) */
	/* 参数: pContext; 云计算对象; */
	/* 参数: szCard; 卡密; */
	/* 参数: szUser; 账号; */
	/* 参数: szPassword; 密码; */
	/* 参数: Info; 绑定信息; 当返回值为true时, 需要调用SP_Cloud_Free进行释放 */
	/* 参数: iError; 云计算错误码 */
	/* 返回: 是否操作成功; 如果出错, 可参考iError  */
	DLLEX bool  SPAPI SP_Cloud_QueryPCSign(void* pContext, OPTIONAL const char* szCard, OUT tagPCSignInfoHead** Info, OPTIONAL int* iError);
	DLLEX bool  SPAPI SP_Cloud_UserQueryPCSign(void* pContext, OPTIONAL const char* szUser, OPTIONAL const char* szPassword, OUT tagPCSignInfoHead** pInfo, OPTIONAL int* iError);

	/* 描述: 通用, 解绑 (每次调用联网) */
	/* 参数: pContext; 云计算对象; */
	/* 参数: char* szCard; 卡密; */
	/* 参数: char* szUser; 账号; */
	/* 参数: char* szPassword; 密码; */
	/* 参数: szPCSign; 机器码; 为空则解绑当前计算机的机器码; */
	/* 参数: u32UnBindIP; 0:解绑时不包含IP, 1:包含IP; */
	/* 参数: iError; 云计算错误码 */
	/* 返回: 是否操作成功; 如果出错, 可参考iError  */
	DLLEX bool  SPAPI SP_Cloud_RemovePCSign(void* pContext, OPTIONAL const char* szCard, const char* szPCSign, unsigned int u32UnBindIP, OPTIONAL int* iError);
	DLLEX bool  SPAPI SP_Cloud_UserRemovePCSign(void* pContext, const char* szUser, OPTIONAL const char* szPassword, const char* szPCSign, unsigned int u32UnBindIP, OPTIONAL int* iError);

	/* 描述: 通用; 获取在线客户端信息 (每次调用联网)  */
	/* 参数: pContext; 云计算对象; */
	/* 参数: char* szCard; 卡密; */
	/* 参数: char* szUser; 账号; */
	/* 参数: char* szPassword; 密码; */
	/* 参数: OUT tagOnlineInfohead* pInfo; 当pInfo不为NULL时, 需要调用SP_Cloud_Free进行释放 */
	/* 参数: iError; 云计算错误码 */
	/* 返回: 是否操作成功; 如果出错, 可参考iError  */
	DLLEX bool  SPAPI SP_Cloud_QueryOnline(void* pContext, OPTIONAL const char* szCard, OUT tagOnlineInfoHead** pInfo, OPTIONAL int* iError);
	DLLEX bool  SPAPI SP_Cloud_UserQueryOnline(void* pContext, const char* szUser, OPTIONAL const char* szPassword, OUT tagOnlineInfoHead** Info, OPTIONAL int* iError);

	/* 描述: 通用; 踢掉在线用户 */
	/* 参数: pContext; 云计算对象; */
	/* 参数: char* szCard; 卡密; */
	/* 参数: char* szUser; 账号; */
	/* 参数: char* szPassword; 密码; */
	/* 参数: int CID; CID */
	/* 参数: iError; 云计算错误码 */
	/* 返回: 是否操作成功; 如果出错, 可参考iError  */
	DLLEX bool  SPAPI SP_Cloud_CloseOnlineByCID(void* pContext, OPTIONAL const char* szCard, unsigned int u32CID, OPTIONAL int* iError);
	DLLEX bool  SPAPI SP_Cloud_UserCloseOnlineByCID(void* pContext, const char* szUser, OPTIONAL const char* szPassword, unsigned int u32CID, OPTIONAL int* iError);

	/* 描述: 通用; 获取试用卡 */
	/* 参数: pContext; 云计算对象; */
	/* 参数: OUT char* szCard[42];// 试用卡 */
	/* 参数: iError; 云计算错误码 */
	/* 返回: 是否操作成功; 如果出错, 可参考iError  */
	DLLEX bool  SPAPI SP_Cloud_ApplyTrialCard(void* pContext, OUT char szCard[42], OPTIONAL int* iError);

	/* 描述: 通用; 账户注册 (每次调用联网) */
	/* 参数: pContext; 云计算对象; */
	/* 参数: char* szUser; 账号; */
	/* 参数: char* szPassword; 密码; */
	/* 参数: char* szSuperPWD; 超级密码; */
	/* 参数: char* szRechargeCards; 充值卡; */
	/* 参数: iError; 云计算错误码 */
	/* 返回: 是否操作成功; 如果出错, 可参考iError  */
	DLLEX bool  SPAPI SP_Cloud_UserRegister(void* pContext, const char* szUser, const char* szPassword, const char* szSuperPWD, const char* szRechargeCards, OPTIONAL int* iError);

	/* 描述: 通用; 账户充值2 (每次调用联网) */
	/* 参数: pContext; 云计算对象; */
	/* 参数: char* szUser; 账号; */
	/* 参数: char* szRechargeCards; 充值卡; */
	/* 参数: tagUserRechargedInfo* pInfo; 充值前后信息; */
	/* 参数: iError; 云计算错误码 */
	/* 返回: 是否操作成功; 如果出错, 可参考iError  */
	DLLEX bool  SPAPI SP_Cloud_UserRecharge(void* pContext, const char* szUser, const char* szRechargeCards, tagUserRechargedInfo* pInfo, OPTIONAL int* iError);

	/* 描述: 通用; 账户修改密码 (每次调用联网) */
	/* 参数: pContext; 云计算对象; */
	/* 参数: char* szUser; 账号; */
	/* 参数: char* szSuperPWD; 超级密码; */
	/* 参数: char* szNewPassword; 新密码; */
	/* 参数: iError; 云计算错误码 */
	/* 返回: 是否操作成功; 如果出错, 可参考iError  */
	DLLEX bool  SPAPI SP_Cloud_UserChangePWD(void* pContext, const char* szUser, const char* szSuperPWD, const char* szNewPassword, OPTIONAL int* iError);

	/* 描述: 通用; 找回密码 (每次调用联网) */
	/* 参数: pContext; 云计算对象; */
	/* 参数: char* szCard; 关联卡/充值卡; */
	/* 参数: OUT char szUser[33]; 账号; */
	/* 参数: OUT char szPassword[33]; 密码; */
	/* 参数: OUT char szSuperPwd[33]; 超级密码; */
	/* 参数: iError; 云计算错误码 */
	/* 返回: 是否操作成功; 如果出错, 可参考iError  */
	DLLEX bool  SPAPI SP_Cloud_RetrievePassword(void* pContext, const char* szCard, OUT char szUser[33], OUT char szPassword[33], OUT char szSuperPWD[33], OPTIONAL int* iError);

	/* 描述: 通用; 获取基本信息 (每次调用联网) */
	/* 参数: pContext; 云计算对象; */
	/* 参数: pBasicInfo; 服务端一些基本配置信息 */
	/* 参数: iError; 云计算错误码 */
	/* 返回: 是否操作成功; 如果出错, 可参考iError  */
	DLLEX bool  SPAPI SP_Cloud_GetBasicInfo(void* pContext, tagBasicInfo* pBasicInfo, OPTIONAL int* iError);

	/* 描述：为了兼容多线程使用封装的申请内存函数 */
	DLLEX void* SPAPI SP_Cloud_Malloc(int iSize);

	/* 描述：为了兼容多线程使用封装的释放内存函数 */
	DLLEX void  SPAPI SP_Cloud_Free(void* pBuff);

	/* 描述: 查询错误码的简略信息; 详细信息参考文件"云计算错误码 详细信息.txt"; (不联网) */
	/* 参数: iError; 云计算错误码 */
	/* 参数: pMsg; 接收错误信息 */
	/* 返回: 是否操作成功; false:没有查询到此错误码信息;  */
	DLLEX bool  SPAPI SP_Cloud_GetErrorMsg(int iError, OUT char szMsg[255]);

#ifdef __cplusplus
}
#endif
