unit MainFormUtils;
{$INCLUDE defines.inc}

interface

uses
  Winapi.Windows, Winapi.Messages, System.SysUtils, System.Variants,
  System.Classes, Vcl.Graphics, Vcl.Controls, Vcl.Forms, Vcl.Dialogs,
  Vcl.ComCtrls, Vcl.ExtCtrls, Vcl.StdCtrls, CallBackUnit, JvComponentBase,
  JvFormPlacement, JvAppStorage, JvAppIniStorage, Vcl.Menus, Vcl.CheckLst,
  Vcl.DBCtrls, siComp;

type
  TMainForm = class(TForm)
    Panel8: TPanel;
    CutProgressLabel: TLabel;
    ProgressLabel: TLabel;
    Button34: TButton;
    ComButton: TButton;
    ComPanel: TPanel;
    PanLed: TShape;
    ComPortBox: TComboBox;
    PageControl: TPageControl;
    TabSheet7: TTabSheet;
    TabSheet9: TTabSheet;
    GroupBox6: TGroupBox;
    Label95: TLabel;
    Label96: TLabel;
    Label97: TLabel;
    StorageEdit15: TEdit;
    StorageEdit16: TEdit;
    StorageEdit17: TEdit;
    LaserPWTrack: TTrackBar;
    Button19: TButton;
    Button52: TButton;
    GroupBox15: TGroupBox;
    Label50: TLabel;
    Label53: TLabel;
    Label54: TLabel;
    Label55: TLabel;
    Label56: TLabel;
    Label57: TLabel;
    Label58: TLabel;
    Label59: TLabel;
    Label94: TLabel;
    Label36: TLabel;
    IStartXEdit: TEdit;
    IStartYEdit: TEdit;
    IWidthEdit: TEdit;
    IHeightEdit: TEdit;
    StorageEdit14: TEdit;
    StorageEdit13: TEdit;
    LoadImageFileBtn: TButton;
    TabSheet2: TTabSheet;
    Label38: TLabel;
    ErrXEdit: TEdit;
    MemGroupBox: TGroupBox;
    Label101: TLabel;
    Label102: TLabel;
    Label113: TLabel;
    Label114: TLabel;
    Label115: TLabel;
    Label116: TLabel;
    ReLoadParmBtn: TButton;
    SaveParmBtn: TButton;
    StorageEdit5: TEdit;
    StorageEdit11: TEdit;
    StorageEdit12: TEdit;
    DefaultParmBtn: TButton;
    ProgressBar: TProgressBar;
    Button2: TButton;
    WorkPanel: TPanel;
    LogMemo: TMemo;
    PLTOpenDialog: TOpenDialog;
    AppIniFileStorage: TJvAppIniFileStorage;
    FormStorage: TJvFormStorage;
    ImageOpenDialog: TOpenDialog;
    MoveTimer: TTimer;
    CheckMouseTimer: TTimer;
    PopupMenu: TPopupMenu;
    N1: TMenuItem;
    N2: TMenuItem;
    N3: TMenuItem;
    Button23: TButton;
    TabSheet3: TTabSheet;
    Button29: TButton;
    CHECKFCPSWBTN: TButton;
    TESTFactoryPassWordEdit: TEdit;
    Label41: TLabel;
    RSTFPSWBTN: TButton;
    Button7: TButton;
    Bevel9: TBevel;
    GBmpCheckBox: TCheckBox;
    UseMemCheckBox: TCheckBox;
    ImageProcModeCheckBox: TCheckBox;
    Label10: TLabel;
    ImageDirectionCBox: TComboBox;
    ReverseCheckBox: TCheckBox;
    GroupBox5: TGroupBox;
    Label69: TLabel;
    Label70: TLabel;
    Label75: TLabel;
    StorageEdit18: TEdit;
    StorageEdit19: TEdit;
    StorageEdit20: TEdit;
    CutPWTrack: TTrackBar;
    Button6: TButton;
    Button11: TButton;
    LPanel: TPanel;
    Label1: TLabel;
    Label2: TLabel;
    Label3: TLabel;
    Label4: TLabel;
    Label5: TLabel;
    Label6: TLabel;
    Label8: TLabel;
    Label9: TLabel;
    Label11: TLabel;
    Label12: TLabel;
    Label14: TLabel;
    PosLabel: TLabel;
    Label16: TLabel;
    Bevel6: TBevel;
    Bevel7: TBevel;
    Bevel8: TBevel;
    Label21: TLabel;
    Label22: TLabel;
    Label23: TLabel;
    Label24: TLabel;
    Label25: TLabel;
    Label26: TLabel;
    Label27: TLabel;
    Label28: TLabel;
    Label29: TLabel;
    Label30: TLabel;
    Label31: TLabel;
    Label33: TLabel;
    Label39: TLabel;
    XMoveToPostEdit: TEdit;
    YMoveToPostEdit: TEdit;
    Button3: TButton;
    Button12: TButton;
    Button13: TButton;
    Button14: TButton;
    Button15: TButton;
    Button16: TButton;
    Button17: TButton;
    Button18: TButton;
    Button20: TButton;
    Button21: TButton;
    XMoveStepEdit: TEdit;
    YMoveStepEdit: TEdit;
    MoveIntervalEdit: TEdit;
    Button22: TButton;
    XMoveToPostEdit1: TEdit;
    YMoveToPostEdit1: TEdit;
    Button24: TButton;
    XMoveToPostEdit2: TEdit;
    YMoveToPostEdit2: TEdit;
    Button25: TButton;
    Button26: TButton;
    XMoveToPostEdit3: TEdit;
    YMoveToPostEdit3: TEdit;
    Button27: TButton;
    Button28: TButton;
    FinishRunComtBox: TComboBox;
    AppendMoreCheckBox: TCheckBox;
    FactroyPanel: TPanel;
    Label105: TLabel;
    Label106: TLabel;
    Label109: TLabel;
    Label110: TLabel;
    Label111: TLabel;
    Label112: TLabel;
    Label117: TLabel;
    Label118: TLabel;
    Label120: TLabel;
    Label119: TLabel;
    Label89: TLabel;
    Label98: TLabel;
    Label17: TLabel;
    Label20: TLabel;
    Label99: TLabel;
    Label100: TLabel;
    Label103: TLabel;
    Label104: TLabel;
    StorageEdit9: TEdit;
    StorageEdit10: TEdit;
    StorageEdit8: TComboBox;
    Button8: TButton;
    StorageEdit21: TEdit;
    StorageEdit22: TEdit;
    MaxWorkSpaceSizeXEdit: TEdit;
    PrinterDrawUnitEdit: TEdit;
    StorageEdit3: TEdit;
    StorageEdit7: TEdit;
    Button35: TButton;
    Button36: TButton;
    Button37: TButton;
    Label44: TLabel;
    Label46: TLabel;
    MaxWorkSpaceSizeYEdit: TEdit;
    Label40: TLabel;
    FactoryPassWordEdit: TEdit;
    Button30: TButton;
    Label35: TLabel;
    OLDFactoryPassWordEdit: TEdit;
    DataCheckListBox: TCheckListBox;
    Button38: TButton;
    Button39: TButton;
    Button40: TButton;
    Button41: TButton;
    Button42: TButton;
    Label34: TLabel;
    Label45: TLabel;
    Label47: TLabel;
    Label48: TLabel;
    StorageEdit23: TEdit;
    StorageEdit24: TEdit;
    Label49: TLabel;
    StorageEdit25: TEdit;
    Label32: TLabel;
    StorageEdit40: TEdit;
    Label79: TLabel;
    Button43: TButton;
    Button44: TButton;
    Label90: TLabel;
    GraphicDPIComBox: TComboBox;
    DefaultDPICheckBox: TCheckBox;
    UnitsComboBox: TComboBox;
    Label52: TLabel;
    Shape2: TShape;
    Label60: TLabel;
    Label61: TLabel;
    Label18: TLabel;
    Label19: TLabel;
    Label42: TLabel;
    Label62: TLabel;
    Label63: TLabel;
    Label64: TLabel;
    Label43: TLabel;
    Label51: TLabel;
    CarveRunSpeedEdit: TEdit;
    XCarveMinSpeedEdit: TEdit;
    CarvePowerEdit: TEdit;
    CarveMinSpeedPowerEdit: TEdit;
    CarveRunSpeedPowerEdit: TEdit;
    Shape1: TShape;
    Label67: TLabel;
    Label68: TLabel;
    Label71: TLabel;
    Label72: TLabel;
    Label65: TLabel;
    Label66: TLabel;
    Label73: TLabel;
    Label74: TLabel;
    Label107: TLabel;
    Label108: TLabel;
    CutMinSpeedEdit: TEdit;
    CutRunspeedEdit: TEdit;
    CutPowerEdit: TEdit;
    CutMinSpeedPowerEdit: TEdit;
    CutRunSpeedPowerEdit: TEdit;
    ZeroPointStyleCheckBox: TCheckBox;
    Label76: TLabel;
    StorageEdit27: TEdit;
    Label77: TLabel;
    CardIDEdit: TEdit;
    Label78: TLabel;
    ActivationCodeEdit: TEdit;
    Button45: TButton;
    Shape4: TShape;
    Label80: TLabel;
    Button46: TButton;
    AutoCheckBox: TCheckBox;
    BinOpenDialog: TOpenDialog;
    Label81: TLabel;
    FirmwareVerLabel: TLabel;
    Button47: TButton;
    UpdaeRadioGroup: TRadioGroup;
    Button49: TButton;
    Button50: TButton;
    Button51: TButton;
    Label82: TLabel;
    Button53: TButton;
    Button54: TButton;
    Button48: TButton;
    Button55: TButton;
    Button56: TButton;
    Label83: TLabel;
    ResetPassWordEdit: TEdit;
    BWCheckBox: TCheckBox;
    Label84: TLabel;
    Label85: TLabel;
    YCarveMinSpeedEdit: TEdit;
    GroupBox1: TGroupBox;
    Button57: TButton;
    Button58: TButton;
    Button59: TButton;
    Label13: TLabel;
    CarveStyleBox: TComboBox;
    Label86: TLabel;
    Label87: TLabel;
    Button10: TButton;
    Button9: TButton;
    StopButton: TButton;
    PauseButton: TButton;
    StartWorkBtn: TButton;
    PICPanel: TPanel;
    GrayImage: TImage;
    Label91: TLabel;
    Label92: TLabel;
    Button60: TButton;
    TrackBar1: TTrackBar;
    Button61: TButton;
    Button62: TButton;
    Button63: TButton;
    Label88: TLabel;
    Label93: TLabel;
    ZMoveStepEdit: TEdit;
    Button64: TButton;
    TabSheet1: TTabSheet;
    GetMainHWND: TButton;
    Button68: TButton;
    Button66: TButton;
    Button67: TButton;
    AdminPanel: TPanel;
    CardIDtoServerBTN: TButton;
    ResetFacPswBTN: TButton;
    SetValuesBTN: TButton;
    OpenDialog1: TOpenDialog;
    siLang: TsiLang;
    siLangDispatcher: TsiLangDispatcher;
    StoragePanel: TPanel;
    Button33: TButton;
    Button1: TButton;
    StorageAddEdit: TEdit;
    Button5: TButton;
    StorageDataEdit: TEdit;
    Button4: TButton;
    Label37: TLabel;
    Button31: TButton;
    Button32: TButton;
    procedure ComButtonClick(Sender: TObject);
    procedure FormCreate(Sender: TObject);
    procedure StartWorkBtnClick(Sender: TObject);
    procedure DefaultParmBtnClick(Sender: TObject);
    procedure SaveParmBtnClick(Sender: TObject);
    procedure ReLoadParmBtnClick(Sender: TObject);
    procedure PauseButtonClick(Sender: TObject);
    procedure StopButtonClick(Sender: TObject);
    procedure Button9Click(Sender: TObject);
    procedure Button10MouseDown(Sender: TObject; Button: TMouseButton; Shift: TShiftState; X, Y: Integer);
    procedure Button10MouseUp(Sender: TObject; Button: TMouseButton; Shift: TShiftState; X, Y: Integer);
    procedure Button4Click(Sender: TObject);
    procedure Button5Click(Sender: TObject);
    procedure Button7Click(Sender: TObject);
    procedure Button34Click(Sender: TObject);
    procedure Button19Click(Sender: TObject);
    procedure Button52Click(Sender: TObject);
    procedure Button6Click(Sender: TObject);
    procedure Button11Click(Sender: TObject);
    procedure FormShow(Sender: TObject);
    procedure Button33Click(Sender: TObject);
    procedure Button1Click(Sender: TObject);
    procedure ImageOpenDialogSelectionChange(Sender: TObject);
    procedure MoveIntervalEditChange(Sender: TObject);
    procedure Button12MouseDown(Sender: TObject; Button: TMouseButton; Shift: TShiftState; X, Y: Integer);
    procedure Button12MouseUp(Sender: TObject; Button: TMouseButton; Shift: TShiftState; X, Y: Integer);
    procedure CheckMouseTimerTimer(Sender: TObject);
    procedure MoveTimerTimer(Sender: TObject);
    procedure StorageEdit5Change(Sender: TObject);
    procedure Button22Click(Sender: TObject);
    procedure Button23Click(Sender: TObject);
    procedure Button3Click(Sender: TObject);
    procedure N1Click(Sender: TObject);
    procedure N3Click(Sender: TObject);
    procedure Button2Click(Sender: TObject);
    procedure Button8Click(Sender: TObject);
    procedure Button28Click(Sender: TObject);
    procedure Button26Click(Sender: TObject);
    procedure XMoveToPostEditKeyPress(Sender: TObject; var Key: Char);
    procedure CutMinSpeedEditKeyPress(Sender: TObject; var Key: Char);
    procedure Button29Click(Sender: TObject);
    procedure Button30Click(Sender: TObject);
    procedure CHECKFCPSWBTNClick(Sender: TObject);
    procedure RSTFPSWBTNClick(Sender: TObject);
    procedure OLDFactoryPassWordEditKeyPress(Sender: TObject; var Key: Char);
    procedure LoadImageFileBtnClick(Sender: TObject);
    procedure Button37Click(Sender: TObject);
    procedure Button36Click(Sender: TObject);
    procedure Button35Click(Sender: TObject);
    procedure Button43Click(Sender: TObject);
    procedure Button44Click(Sender: TObject);
    procedure Button45Click(Sender: TObject);
    procedure Button46Click(Sender: TObject);
    procedure CutPWTrackChange(Sender: TObject);
    procedure LaserPWTrackChange(Sender: TObject);
    procedure Panel8Click(Sender: TObject);
    procedure Button49Click(Sender: TObject);
    procedure Button50Click(Sender: TObject);
    procedure Button51Click(Sender: TObject);
    procedure Button53Click(Sender: TObject);
    procedure Button54Click(Sender: TObject);
    procedure Button48Click(Sender: TObject);
    procedure Button55Click(Sender: TObject);
    procedure Button56Click(Sender: TObject);
    procedure Button57Click(Sender: TObject);
    procedure Button58Click(Sender: TObject);
    procedure Button59Click(Sender: TObject);
    procedure Button60Click(Sender: TObject);
    procedure TrackBar1Change(Sender: TObject);
    procedure Button61MouseDown(Sender: TObject; Button: TMouseButton; Shift: TShiftState; X, Y: Integer);
    procedure Button64Click(Sender: TObject);
    procedure Button66Click(Sender: TObject);
    procedure GetMainHWNDClick(Sender: TObject);
    procedure Button68Click(Sender: TObject);
    procedure CardIDtoServerBTNClick(Sender: TObject);
    procedure ActivationCodeEditDblClick(Sender: TObject);
    procedure ResetFacPswBTNClick(Sender: TObject);
    procedure Button31Click(Sender: TObject);
  private
    { Private declarations }
    function LoadSysComList(const Com: string = ''): Integer; //列出COM口
    //
    procedure vProgress(const vPostion: Integer; const vTotalCount: Integer); cdecl;//进度条回调
    procedure vSysMessage(const SysMsgIndex: Integer; const SysMsgCode: Integer; const SysEventData: PChar); cdecl;//信息回调
    //
    procedure LoadJsonFile(const FileName: TFileName); //导入Json
    procedure LoadBinFile(const FileName: TFileName); //导入Bin
    procedure LoadSvgFile(const FileName: TFileName); //导入SVG
    procedure LoadPltFile(const FileName: TFileName); //导入PLT
    procedure LoadPicFile(const FileName: TFileName); //导入图像
    //
    procedure ShowUpdateAskWindow(//发现新版本，弹窗询问是否升级
      const IsFirmwareUpdate: Boolean; //是否固件更新，否为软件更新
      const Flag: string; //标识符
      const vJsonFile: string; //
      const BeforeRunType: Integer; //升级前运行的程序运行方式 0运行并等待结束 1运行后无需等待继续执行升级(默认)
      const BeforeRunExeName: string; //升级开始前运行的程序文件名，为空则升级完不运行
      const AfterRunExeName: string //升级完成后运行的程序文件名，为空则升级完不运行
    );
  public
    { Public declarations }
  end;

var
  MainForm: TMainForm;
//=====================================================================================================
  vLanguage: Integer = 0;
  cApplicationPath: string = ''; //APP所在路径

  vAutoWorkCount: Integer = 0;
  ProcStr: string = '';
  CurrentVersion: Integer = 0; //当前版本号

  IsFirmwareUpdate: Boolean; //是否固件更新，否为软件更新
  BeforeRunExeName: string; //升级前运行的程序
  AfterRunExeName: string; //升级后运行的程序
  JsonFile: string; //升级日志文件名
  //**********************************************
  XStepNum: Double; //为 X 轴脉冲长度 一个脉冲移动的距离 单位mm
  YStepNum: Double; //为 Y 轴脉冲长度 一个脉冲移动的距离 单位mm

  RetOriginalSpeed: Double = 200; //回原点速度
  HStep: Word; //行间距
  LStep: Word; //列间距:像素间隔
  //**********************************************
  vPageCount: Integer = 0; //包数量
  //
  MinSpeed: Word = 20; //为实际设定初始速度 电机启动运行时的起始速度，该速度值过小将导致启动缓慢，过大将导致电机启动丢步
  Runspeed: Word = 100; //为实际设定运行速度 电机完成加速过程后的工作速度
  EndSpeed: Word = 20; //为实际设定结点速度 完成一条线段时结束点的速度，电机到达该位置前，将根据该速度提前减速
  //
  MoveMinSpeed: Integer = 200; //快速移动起跳速度
  MoveWorkSpeed: Integer = 200; //为实际设定的空移速度: Word; //快速移动速度

  MoveToX, MoveToY, MoveToZ: Double; //当前坐标...单位mm
  WorkArea: Byte = 1; //工作象限
  WorkSpaceWidth, WorkSpaceHeight: Word; //加工幅面宽高
  //
  WorkState: Integer = 2; //工作状态
  IsRequestAndContinueDLGshow: Boolean = False; //请求断点续传的DLG是否已经显示
  //
  XMoveToPos1: Double = 0;
  YMoveToPos1: Double = 0; //第 1 组加工原点
  XMoveToPos2: Double = 0;
  YMoveToPos2: Double = 0; //第 2 组加工原点
  XMoveToPos3: Double = 0;
  YMoveToPos3: Double = 0; //第 3 组加工原点
//=====================================================================================================

const
{$IFDEF ENGLISHVER}
  cCaption: array[0..2] of string = ('Pause', 'Continue', 'Pause');
  cLaserStr: array[5..6] of string = ('Laser OFF', 'Laser ON');
  CapStr: array[0..1] of string = ('Connect', 'Dis Connect');
  UPD: array[Boolean] of string = ('software ', ' firmware ');
  ReturMsg: array[-4..0] of string = ('Too many packets', 'data conversion error', 'too many pixels', ' conversion failure ', ' conversion failure ');
  //
  Titel = 'Tips';
  InsertUsbDog = 'Please insert the encryption lock!';
  OutSize = 'Beyond the maximum processing size!';
  AskContinue = 'Currently, there are unfinished tasks. Do you want to continue processing?';
  RegisterOK = 'Board registration successful';
  RegisterError = 'Board registration failed';
  StartWorTip = '%d packets sent / Total %d packets';
  StartUpdateTip = '%d Kb / Total %d Kb';
  DownLoadFileCountsTip = 'There are %s files to update';
  DownLoadFileIndexTip = 'Ready to update file %s >> %s';
  FactroyPasswordInValidTip = 'Manufacturer password error';
  ChangeFactroyPasswordOKTip = 'Factory password modified successfully';
  ChangeFactroyPasswordErrorTip = 'Factory password modification failed';
  MainCardIDErrorTip = 'Invalid board ID!';
  EnLockerIDErrorTIP = 'Invalid encryption lock ID!';
  IsLatestVersionTip = 'The current version is up to date';
  SoftUpdateFailedTip = 'Software update failed';
  UpdateFirmwareEndTip = 'Board firmware upgrade successful!';
  TimeConsumingTip = 'The total time consumption of this processing task is as follows:%s';
  WorkAreaTip = 'The current working quadrant is quadrant %d';
  RegisterMainCardTip = 'After registration, please power on the board again!';
  FoundNewVersion = 'A new version of %s has been launched, version number: %d ' + #13#10#13#10 + '%s' + #13#10#13#10 + 'Update now? ';
  UpdateTip = 'Online upgrade';
  ResoreTip = 'Do you want to restore the parameter to the default value?';
  CopyToClipboardTip = 'Card ID has been copied to the clip board!';
  RestoreParamTip = 'Do you want to restore the above parameters to the default values?';
  CardActivedTip = 'Activated!';
  CardRegDateTip = 'Board registration date:%s';
  CardAciveDateTip = 'Board activation date：%s';
  MachineRegDateTip = 'Machine registration date：%s';
  MachineAciveDateTip = 'Machine activation date：%s';
  MachineMaintenanceTimesTip = 'Machine maintenance times：%s';
  InvalidMachineIDTip = 'The machine ID entered during activation is invalid...';
{$ELSE}
  cCaption: array[0..2] of string = ('暂停', '继续', '暂停');
  cLaserStr: array[5..6] of string = ('关激光', '开激光');
  CapStr: array[0..1] of string = ('联机', '脱机');
  ReturMsg: array[-4..0] of string = ('数据分包数过大', '数据转换错误', '像素数过大', '转换失败', '转换失败');
  //
  Titel = '提示';
  InsertUsbDog = '请插入加密锁！';
  OutSize = '超出最大可加工幅面！';
  AskContinue = '当前有未加工完成的任务，要继续加工吗？';
  RegisterOK = '板卡注册成功';
  RegisterError = '板卡注册失败';
  StartWorTip = '%d 包已发 / 共 %d 包';
  StartUpdateTip = '%d Kb / 共 %d Kb';
  DownLoadFileCountsTip = '共有 %s 个文件需要更新...';
  DownLoadFileIndexTip = '准备更新第 %s 个文件 >> %s';
  FactroyPasswordInValidTip = '厂家密码错误';
  ChangeFactroyPasswordOKTip = '厂家密码修改成功';
  ChangeFactroyPasswordErrorTip = '厂家密码修改失败';
  MainCardIDErrorTip = '板卡 ID 无效！';
  EnLockerIDErrorTIP = '加密锁 ID 无效！';
  IsLatestVersionTip = '当前版本已经最新';
  SoftUpdateFailedTip = '软件更新失败';
  UpdateFirmwareEndTip = '板卡固件升级成功！';
  TimeConsumingTip = '本次加工任务总计耗时:%s';
  WorkAreaTip = '当前工作象限为 第 %d 象限';
  RegisterMainCardTip = '注册完成后请给板卡重新上电！';
  UPD: array[Boolean] of string = ('软件', '固件');
  FoundNewVersion= '%s已有新版本推出，版本号:%d' + #13#10#13#10 + '%s' + #13#10#13#10 + '是否立即更新？';
  UpdateTip='在线升级';
  ResoreTip ='要把参数恢复为默认值吗？';
  CopyToClipboardTip = '板卡ID已经复制到剪贴版！';
  RestoreParamTip = '要把上述参数恢复为默认值吗？';
  CardActivedTip= '已激活！';
  CardRegDateTip='板卡注册日期：%s';
  CardAciveDateTip ='板卡激活日期：%s';
  MachineRegDateTip ='机器注册日期：%s';
  MachineAciveDateTip ='机器激活日期：%s';
  MachineMaintenanceTimesTip ='机器维护次数：%s';
  InvalidMachineIDTip ='激活时输入的机器ID无效...';
{$ENDIF}

  vLine = '-------------------------------------------------------------------------------------';
  cOpen: array[Boolean] of TColor = (clBlack, clRed); //串口状态
  cWorkAreaX: array[1..4] of Integer = (1, -1, -1, 1); //1-4象限X符号
  cWorkAreaY: array[1..4] of Integer = (1, 1, -1, -1); //1-4象限Y符号

  cFlag: array[0..2] of string = (//
    '{7ADDA8F6-69A6-410A-B17F-260733FA50D5}', //本地固件
    '{4A5F9C85-8735-414D-BCA7-E9DD111B23A8}', //内部测试
    '{CC9094BA-2991-4CE2-A514-BF0EA3685D05}'//公测版
    );
//=====================================================================================================

{$INCLUDE CompileInfo.inc}
{$DEFINE WRITELOG} //开启日志输出>简要

implementation

uses
  LaserLibUtils, System.StrUtils, Winapi.ShellAPI, UpdateFirmwareFormUtils,
  ActiveMBFormUtils, qjson, System.IOUtils, Cromis.Client, Simple.Loger;
{$R *.dfm}

function BinToDec(Value: string): Integer; //二进制转化为十进制
var
  str: string;
  i: Integer;
begin
  str := UpperCase(Value);
  Result := 0;
  for i := 1 to Length(str) do
    Result := Result * 2 + Ord(str[i]) - 48;
end;

//日志相关==================================================================================================

procedure TMainForm.N1Click(Sender: TObject); //清空日志
begin
  ClearLog; //清空日志
end;

procedure TMainForm.N3Click(Sender: TObject);  //保存日志
begin
  SaveLogToFile; //保存日志
end;

//日志相关==================================================================================================

//COM操作相关==================================================================================================

function GetComPortName(const ComStr: string): Word; //提取COM序号
var
  LeftPos, RightPos: Word;
begin
  if Trim(ComStr) <> '' then
  begin
    LeftPos := Pos('(COM', ComStr) + 4;
    RightPos := Pos(')', ComStr);

    Result := StrToIntDef(Copy(ComStr, LeftPos, RightPos - LeftPos), 0);
  end;
end;

function TMainForm.LoadSysComList(const Com: string = ''): Integer;  //读取COM列表并自动连接第一个
var
  i: Byte;
  ComStr: string;
  ComPortLst: TArray<string>;
begin
  Result := 0;

  with ComPortBox do
  begin
    Items.Clear;

    if Trim(Com) <> '' then
      ComStr := Com
    else
      ComStr := GetComPortList; //加载COM列表

    if Trim(ComStr) <> '' then
    begin
      ComPortLst := ComStr.Split([';']);
      for i := Low(ComPortLst) to High(ComPortLst) do
        if Items.IndexOf(ComPortLst[i]) = -1 then
        begin
          Items.Append(ComPortLst[i]);
        end;

      Result := Items.Count;
      if Result > 0 then
      begin
        ItemIndex := 0;

        InitComPort(GetComPortName(ComPortBox.Text)); //1表示com1，类推
      end
    end;
  end;
end;

//COM操作相关==================================================================================================

//系统回调相关==================================================================================================

procedure TMainForm.vProgress(const vPostion, vTotalCount: Integer);
begin
  ProgressBar.Position := vPostion + 1;
  ProgressBar.Max := vTotalCount;
  ProgressLabel.Caption := Format(ProcStr, [vPostion, vTotalCount]);
end;

procedure TMainForm.vSysMessage(const SysMsgIndex, SysMsgCode: Integer; const SysEventData: PChar);
var
  vTms: string;
  vAddr: Byte;
  vData: Single;
  i: Word;
  vEventData: TArray<string>;
  //工作状态，执行的指令，X坐标，Y坐标，Z坐标，激光功率，灰度值
  iWorkState, iCmdCode, iLaserPower, iGray: Integer;
  iXPos, iYPos, iZPos: Double;
  //
  vUserName: PChar; //姓名
  vAddress: PChar; //地址
  vTelphone: PChar; //电话
  vQQ: PChar; //QQ
  vWX: PChar; //WX
  vEmail: PChar; //邮箱
  Country: PChar; //国家
  Distributor: PChar; //经销商
  Trademark: PChar; //品牌
  Model: PChar; //型号
  MachineID: PChar; //机器ID
begin
//--------------------------------------------------------------------------------------------------------------------------
  WriteRZ(Format(' SysMsgIndex = %d SysMsgCode = %d SysEventData = %s', [SysMsgIndex, SysMsgCode, SysEventData]), Now); //
  WriteRZ(vLine, Now, False);
//--------------------------------------------------------------------------------------------------------------------------
  case SysMsgCode of
    EnLockerNotExists: //加密锁不存在
      begin
        FactroyPanel.Visible := False;
        Application.MessageBox(InsertUsbDog, Titel, MB_OK + MB_ICONSTOP + MB_TOPMOST);
      end;
    USBArrival, USBRemove: //USB设备已连
      begin

      end;
    GetComPortListOK: //获取列表成功
      begin
        LoadSysComList(SysEventData); //列出COM口
      end;
    MachineOffLine:
      begin
        ComPortBox.Items.Clear;
      end;
    ComPortOpened:
      begin
        with ComButton do
        begin
          Tag := 1;
          Caption := CapStr[Tag]; //显示：脱机
        end;
        //
        FactroyPanel.Visible := False;
        //
        AppIniFileStorage.Reload;
        PanLed.Brush.Color := cOpen[True];
        PageControl.Enabled := True;
        WorkPanel.Enabled := True;
        ComPanel.Enabled := False;
      end;
    ComPortClosed:
      begin
        with ComButton do
        begin
          Tag := 0;
          Caption := CapStr[Tag]; //显示：联机
        end;
        //
        AppIniFileStorage.Flush;
        PanLed.Brush.Color := cOpen[False];
        PageControl.Enabled := False;
        WorkPanel.Enabled := False;
        ComPanel.Enabled := True;
        FactroyPanel.Visible := False;
        CardIDEdit.Text := '';
      end;
    ReTransTimeOutError, TransTimeOutError, DataFormatError, TransCompleteOK:
      begin
        StartWorkBtn.Enabled := True;
        LoadImageFileBtn.Enabled := True;
      end;
    GraphicMaxSizeError: //加工范围超限
      begin
        ShowMessage(OutSize);
      end;
    RequestAndContinue: //板卡请求续传，是否继续
      if not IsRequestAndContinueDLGshow then
      begin
        IsRequestAndContinueDLGshow := True;

        //调用继续加工的接口
        LoadBreakPiontData(Application.MessageBox(AskContinue, Titel, MB_YESNO + MB_ICONQUESTION + MB_DEFBUTTON2 + MB_TOPMOST) = ID_YES); //载入断点续传数据
      end;
    ContinueWorking: //继续加工
      begin
        PauseButton.Caption := cCaption[0];
        PauseButton.Tag := 0;
      end;
    MainCardRegisterOK: //板卡注册成功
      begin
        Label80.Caption := RegisterOK;
        CardIDtoServerBTN.Enabled := True;
      end;
    MainCardRegisterError: //板卡注册失败
      begin
        Label80.Caption := RegisterError;
        CardIDtoServerBTN.Enabled := False;
      end;
    StartWorking:
      ProcStr := StartWorTip;
    DownloadFirmwareDataStart, StartSoftUpdate, DownloadSoftDataStart: //
      ProcStr := StartUpdateTip;
    DownLoadFileCounts:
      WriteRZ(Format(DownLoadFileCountsTip, [SysEventData]), Now);
    DownLoadFileIndex:
      try
        vTms := SysEventData;
        vEventData := vTms.Split([';']); //以;分隔 格式为： 文件序号;文件相对路径及名称
        WriteRZ(Format(DownLoadFileIndexTip, [vEventData[0], vEventData[1]]), Now);
      except
      end;
    MainCardIsPirate: //未激活，需要激活
      begin
        vTms := '';
        try
          if not Assigned(ActiveMBForm) then
            ActiveMBForm := TActiveMBForm.Create(nil);

          with ActiveMBForm do
            case ShowModal of
              mrOk:
                try
                  {
                    const vUserName: PChar; //姓名
                    const vAddress: PChar; //地址
                    const vTelphone: PChar; //电话
                    const vQQ: PChar; //QQ
                    const vWX: PChar; //WX
                    const vEmail: PChar; //邮箱
                    const Country: PChar; //国家
                    const Distributor: PChar; //经销商
                    const Trademark: PChar; //品牌
                    const Model: PChar; //型号
                    const MachineID: PChar//机器ID
                  }
                  vUserName := PChar(UserNameEdit.Text); //姓名
                  vAddress := PChar(AddressEdit.Text); //地址
                  vTelphone := PChar(TelphoneEdit.Text); //电话
                  vQQ := PChar(QQEdit.Text); //QQ
                  vWX := PChar(WXEdit.Text); //WX
                  vEmail := PChar(EmailEdit.Text); //邮箱
                  Country := PChar(CountryEdit.Text); //国家
                  Distributor := PChar(DistributorEdit.Text); //经销商
                  Trademark := PChar(TrademarkEdit.Text); //品牌
                  Model := PChar(ModelEdit.Text); //型号
                  MachineID := PChar(MachineIDEdit.Text); //机器ID
                  vTms := 'mrOk';
                except
                  vTms := '';
                end;
              mrRetry:
                vTms := 'mrRetry'; //
            end;
        finally
          if Assigned(ActiveMBForm) then
            FreeAndNil(ActiveMBForm);
        end;
        //
        if vTms = 'mrOk' then
          ActivationMainCard(//
            vUserName, //姓名
            vAddress, //地址
            vTelphone, //电话
            vQQ, //QQ
            vWX, //WX
            vEmail, //邮箱
            Country, //国家
            Distributor, //经销商
            Trademark, //品牌
            Model, //型号
            MachineID //机器ID
          )
        else if vTms = 'mrRetry' then
        begin
          GetMainCardInfo;
        end;
      end;
    MainCardMachineMoreInfo: //返回板卡及机器注册及激活信息
      begin
        //2020/9/1 13:07:42;2020/9/12 23:26:38;2020/9/12 21:03:26;2020/9/12 23:26:38;123063
        vTms := Trim(SysEventData);

        vEventData := vTms.Split([';']); //以;分隔

        vTms := '';
        if vEventData[1] <> '' then
          vTms := CardActivedTip + #13#10 + #13#10 + //
            '----------------------------------------------';
        vTms := vTms + #13#10 + #13#10 + //
          Format(CardRegDateTip, [vEventData[0]]) + #13#10 + #13#10 + //
          Format(CardAciveDateTip, [vEventData[1]]) + #13#10 + #13#10 + //
          '----------------------------------------------' + #13#10 + #13#10 + //
          Format(MachineRegDateTip, [vEventData[2]]) + #13#10 + #13#10 + //
          Format(MachineAciveDateTip, [vEventData[3]]) + #13#10 + #13#10 + //
          Format(MachineMaintenanceTimesTip, [vEventData[4]]);

        if vEventData[2] = '' then
          vTms := vTms + #13#10 + #13#10 + //
            '----------------------------------------------' + #13#10 + #13#10 + //
            InvalidMachineIDTip;

        ShowMessage(vTms);
      end;
    ReturnWorkState: //返回当前工作状态
      begin
        //工作状态，执行的指令，X坐标，Y坐标，Z坐标，激光功率，灰度值
        //iWorkState, iCmdCode, iXPos, iYPos, iZPos, iLaserPower, iGray: Integer;
        vTms := Trim(SysEventData);

        try
          vEventData := vTms.Split([';']); //以;分隔

          iWorkState := StrToIntDef(vEventData[0], 0); //工作状态
          iCmdCode := StrToIntDef(vEventData[1], 0); //执行的指令
          iXPos := StrToFloatDef(vEventData[2], 0.0); //X坐标
          iYPos := StrToFloatDef(vEventData[3], 0.0); //Y坐标
          iZPos := StrToFloatDef(vEventData[4], 0.0); //Z坐标
          iLaserPower := StrToIntDef(vEventData[5], 0); //激光功率
          iGray := StrToIntDef(vEventData[6], 0); //灰度值

          WorkState := iCmdCode;
          MoveToX := iXPos; //Round(iXPos / XStepNum);
          MoveToY := iYPos; //Round(iYPos / XStepNum);
          MoveToZ := iZPos;
          PosLabel.Caption := Format('X = %g Y = %g Z = %g', [MoveToX, MoveToY, MoveToZ]);
          PauseButton.Caption := cCaption[iWorkState];
          PauseButton.Tag := iWorkState;
        except
        end;

        if CardIDEdit.Text = '' then
        begin
          CardIDEdit.Text := GetMainCardID; //返回板卡ID
        end;
      end;
    FactroyPasswordValid: //厂家密码正确
      begin
        FactroyPanel.Visible := True;
      end;
    FactroyPasswordInValid: //厂家密码错误
      begin
        FactroyPanel.Visible := False;
        ShowMessage(FactroyPasswordInValidTip);
      end;
    ChangeFactroyPasswordOK:
      begin
        ShowMessage(ChangeFactroyPasswordOKTip);
      end;
    ChangeFactroyPasswordError:
      begin
        ShowMessage(ChangeFactroyPasswordErrorTip);
      end;
    MainCardIDError:
      begin
        ShowMessage(MainCardIDErrorTip);
      end;
    EnLockerIDError:
      begin
        ShowMessage(EnLockerIDErrorTIP);
      end;
    IsLatestVersion: //
      ShowMessage(IsLatestVersionTip);
    SoftUpdateFailed: //
      ShowMessage(SoftUpdateFailedTip);
    FoundSoftNewVersion, FoundFirmwareNewVersion: //发现新版本
      begin
        IsFirmwareUpdate := SysMsgCode = FoundFirmwareNewVersion; //是固件吗
        //
        //ShowMessage(JsonFile);
        ShowUpdateAskWindow(//发现新版本，弹窗询问是否升级
          IsFirmwareUpdate, //
          PChar(cFlag[UpdaeRadioGroup.ItemIndex]), //标识符
          JsonFile, //升级日志文件
          0, //升级前运行的程序运行方式 0运行并等待结束 1运行后无需等待继续执行升级(默认)
          BeforeRunExeName, //升级前要运行的程序，支持.BAT .EXE
          AfterRunExeName //升级完要启动的EXE
        );
      end;
    SendFirmwareDataEnd:
      begin
        if not Assigned(UpdateFirmwareForm) then
        begin
          UpdateFirmwareForm := TUpdateFirmwareForm.Create(Self);
          with UpdateFirmwareForm do
          begin
            Show;
            SetWindowPos(Handle, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE or SWP_NOSIZE);
          end;
        end;
      end;
    UpdateFirmwareEnd: //固件更新成功
      begin
        if Assigned(UpdateFirmwareForm) then
          FreeAndNil(UpdateFirmwareForm);

        WriteRZ(UpdateFirmwareEndTip, Now);
      end;
    UpdateFirmwareTimeOut: //升级超时
      begin
        if Assigned(UpdateFirmwareForm) then
          FreeAndNil(UpdateFirmwareForm);
      end;
    WorkFinished: //加工完成
      begin
        Inc(vAutoWorkCount);
        WriteAutoWorkInfo(Format('第 %d 次加工完成，准备再次重复加工...', [vAutoWorkCount]));
        if AutoCheckBox.Checked then
          LoadImageFileBtn.Click;
      end;
    TimeConsuming: //加工任务总计耗时时长
      begin
        WriteRZ(Format(TimeConsumingTip, [API_FormatMillisecondString(StrToIntDef(SysEventData, 0))]), Now);
      end;
    ReadParamsFromServerOK, ReadSysParamFromCardOK, WriteSysParamToCardOK: //读取到的寄存器内容 返回格式为：地址1,数据1;地址2,数据2;...地址n,数据n;
      begin
        vTms := Trim(SysEventData);

        try
          vEventData := vTms.Split([';']); //以;分隔

          for i := Low(vEventData) to High(vEventData) - 1 do
          begin
            vAddr := StrToIntDef(Format('%s', [LeftStr(vEventData[i], Pos(',', vEventData[i]) - 1)]), 300); //地址

            if vAddr in [1..128] then //
            begin
              vTms := Format('%s', [RightStr(vEventData[i], Length(vEventData[i]) - Pos(',', vEventData[i]))]);
              vData := StrToFloatDef(vTms, 0); //数据
              //
              case vAddr of  //应用到变量
                2: //固件版本
                  FirmwareVerLabel.Caption := GetMainHardVersion; //固件版本
                4: //雕刻时Y轴起跳速度
                  YCarveMinSpeedEdit.Text := vTms;
                5:
                  MoveWorkSpeed := Round(vData); //快速移动速度
                7:
                  RetOriginalSpeed := Round(vData); //回原点速度
                8:
                  try
                    WorkArea := Round(vData); //工作象限
                    StorageEdit8.ItemIndex := WorkArea - 1;
                    Label16.Caption := Format(WorkAreaTip, [WorkArea]);
                  except
                  end;
                9:
                  begin
                    XStepNum := (vData); //X 轴脉冲长度
                  end;
                10:
                  begin
                    YStepNum := (vData); //Y 轴脉冲长度
                  end;
                13:
                  begin
                    LStep := Round(vData); //列间距:水平像素间隔
                  end;
                14:
                  begin
                    HStep := Round(vData); //行间距：垂直像素间隔
                  end;
                31: //第 1 组加工原点 X
                  begin
                    XMoveToPos1 := (vData); //
                    //
                    XMoveToPostEdit1.Text := vTms; //FloatToStr(vData);
                  end;
                32:  //第 1 组加工原点 Y
                  begin
                    YMoveToPos1 := (vData); //
                    //
                    YMoveToPostEdit1.Text := vTms; //FloatToStr(vData);
                  end;
                33:  //第 2 组加工原点 X
                  begin
                    XMoveToPos2 := (vData); //
                    //
                    XMoveToPostEdit2.Text := vTms; //FloatToStr(vData);
                  end;
                34:  //第 2 组加工原点 Y
                  begin
                    YMoveToPos2 := (vData); //
                    //
                    YMoveToPostEdit2.Text := vTms; //FloatToStr(vData);
                  end;
                35:   //第 3 组加工原点 X
                  begin
                    XMoveToPos3 := (vData); //
                    //
                    XMoveToPostEdit3.Text := vTms; //FloatToStr(vData);
                  end;
                36:  //第 3 组加工原点 Y
                  begin
                    YMoveToPos3 := (vData); //
                    //
                    YMoveToPostEdit3.Text := vTms; //FloatToStr(vData);
                  end;
                38:
                  begin    //加工幅面宽高
                    WorkSpaceWidth := Round(vData) div 256 div 256; //高位
                    WorkSpaceHeight := Round(vData) and $FFFF; //低位
                    //
                    MaxWorkSpaceSizeXEdit.Text := IntToStr(WorkSpaceWidth);
                    MaxWorkSpaceSizeYEdit.Text := IntToStr(WorkSpaceHeight);
                  end;
                39:
                  begin
                    PrinterDrawUnitEdit.Text := IntToStr(Round(vData)); //绘图单位
                  end;
                40:
                  MoveMinSpeed := Round(vData); //快速移动起跳速度
              end;
              //
              StorageDataEdit.Text := vTms;
              if vAddr in [3..25, 27..128] then
                TEdit(Self.FindComponent(Format('StorageEdit%d', [vAddr]))).Text := vTms; //
              //
            end;
          end;
        except
        end;
      end;
  end;
end;

procedure TMainForm.XMoveToPostEditKeyPress(Sender: TObject; var Key: Char);
begin
  if not (Key in ['0'..'9', '.', #8, #13]) then
    Key := #0;
end;

//系统回调相关==================================================================================================

procedure TMainForm.FormCreate(Sender: TObject);
var
  TmStr: string;
const
  AppTitel = 'LaserLib API Demo';
begin
  Randomize;
  //-----------------------
  DoubleBuffered := True;
  //-----------------------
  CurrentVersion := API_GetVertionNumber(CompileDT); //把编译日期转为版本号
  {$IFDEF DEBUG}
  TmStr := 'Debug';
  {$ELSE}
  TmStr := 'Release';
  {$ENDIF}  //
  {$IFDEF MSWINDOWS}
  {$IFDEF CPUX64}
  Caption := Format('%s x64_%s Compile:%s v%d / API %s v%d', [AppTitel, TmStr, CompileDT, CurrentVersion, GetAPILibCompileDT, API_GetVertionNumber(GetAPILibCompileDT)]);
  {$ELSE}
  Caption := Format('%s x32_%s Compile:%s v%d / API %s v%d', [AppTitel, TmStr, CompileDT, CurrentVersion, GetAPILibCompileDT,API_GetVertionNumber(GetAPILibCompileDT)]);
  {$ENDIF}  {$ENDIF}
  //
  InitLoger(MainForm.LogMemo.Lines);
  //
  InitLib(Self.Handle);
  //注册回调函数
  ProgressCallBack(vProgress);
  SysMessageCallBack(vSysMessage);
  //
  try
    siLang.LoadAllFromFile(ExtractFilePath(ParamStr(0)) + 'Languge.sil', True);
  {$IFDEF ENGLISHVER}
    SetLanguge(0); //英文
    SetFactroyType('DemoEng'); //设置英文版DEMO专属名称(匹配升级通道用)

    siLangDispatcher.Language := siLangDispatcher.LangNames[1];
    StoragePanel.Free;
  {$ELSE}
    SetLanguge(1); //中文
    SetFactroyType('Demo'); //设置中文版DEMO专属名称(匹配升级通道用)

    siLangDispatcher.Language := siLangDispatcher.LangNames[0];
  {$ENDIF}
  except
  end;
  //
  {$IFDEF SUPPERADMIN}
  AdminPanel.Visible := True;
  {$ELSE}
  FreeAndNil(AdminPanel);
  {$ENDIF}  //
  LogMemo.Lines.Append(GetAPILibCompileInfo); //获取API的编译模式
  PageControl.ActivePageIndex := 1;
  //
  cApplicationPath := ExtractFilePath(ParamStr(0));
  BeforeRunExeName := PChar(Format('%s\BeforeRun.bat', [ExtractFilePath(ParamStr(0))])); //升级前运行(可选)
  AfterRunExeName := PChar(ParamStr(0)); //升级后运行(一般为主EXE)
  JsonFile := PChar(Format('%sVersionLog.Json', [TPath.GetTempPath])); //保存在TEMP文件夹下
end;

procedure TMainForm.FormShow(Sender: TObject);
begin
  LoadSysComList; //列出COM口
end;

procedure TMainForm.GetMainHWNDClick(Sender: TObject);
var
  vCDRHwnd: Integer; //CDR主窗口句柄
begin
  {示例：如何把CDR主窗口置为正常状态}
  //本库提供的函数，获取CDR主窗口实例的句柄
  vCDRHwnd := GetCDRMainFormHWND(PChar('D:\Program Files\Corel\CorelDRAW Graphics Suite X8\Programs64\CorelDRW.exe'));

  if vCDRHwnd > 0 then //CDR主窗口句柄窗口获取成功
  begin
    //第一步，显示窗口
    ShowWindow(//系统API，该函数设置指定窗口的显示状态
      vCDRHwnd, //CDR主窗口句柄
      SW_NORMAL//窗口的显示方式为：正常状态(无论是否显示或隐藏于任务栏，一律恢复到桌面显示)
    );

    //第二步，窗口提前
    BringWindowToTop(//系统API，该函数将指定的窗口设置到Z序的顶部。如果窗口为顶层窗口，则该窗口被激活；如果窗口为子窗口，则相应的顶级父窗口被激活。
      vCDRHwnd //CDR主窗口句柄
    ); //
  end
  else //获取失败后，返回值
    case vCDRHwnd of
      0:
        ShowMessage('获取 CDR 主窗口句柄失败！');
      -1:
        ShowMessage('指定EXE文件不存在！');
      -2:
        ShowMessage('指定EXE文件没有运行！');
    end;
end;

procedure TMainForm.ImageOpenDialogSelectionChange(Sender: TObject);
begin
  with TOpenDialog(Sender) do
  begin
    InitialDir := ExtractFilePath(FileName)
  end;
end;

//基本控制相关==================================================================================================
procedure TMainForm.StartWorkBtnClick(Sender: TObject); //开始加工
begin
  if vPageCount > 0 then
  begin
    PauseButton.Caption := cCaption[0]; //'暂停';
    PauseButton.Tag := 0;

    StartMachining(not ZeroPointStyleCheckBox.Checked); //加工原点定义：True:机械原点为原点 False:当前坐标点为原点;
  end;
end;

procedure TMainForm.Panel8Click(Sender: TObject);
begin
  vAutoWorkCount := 0;
end;

procedure TMainForm.PauseButtonClick(Sender: TObject); //暂停继续
begin
  with TButton(Sender) do
  begin
    if Tag = 0 then
      Tag := 1
    else
      Tag := 0;

    Caption := cCaption[Tag];
    PauseContinueMachining(Tag = 0);
  end;
end;

procedure TMainForm.StopButtonClick(Sender: TObject);  //停止加工
begin
  StopMachining;
  GrayImage.Picture.Graphic := nil;
  ProgressBar.Position := 0;
end;

procedure TMainForm.Button9Click(Sender: TObject); //回原点   RetOriginalSpeed
begin
  LPenMoveToOriginalPoint(RetOriginalSpeed);
end;

procedure TMainForm.ActivationCodeEditDblClick(Sender: TObject);
begin
  ActivationCodeEdit.PasteFromClipboard; //双击复制粘贴板内容
end;

procedure TMainForm.Button10MouseDown(Sender: TObject; Button: TMouseButton; Shift: TShiftState; X, Y: Integer); //点射
begin
  TestLaserLight(True);  //开激光
end;

procedure TMainForm.Button10MouseUp(Sender: TObject; Button: TMouseButton; Shift: TShiftState; X, Y: Integer);  //点射
begin
  TestLaserLight(False);  //关激光
end;

procedure TMainForm.Button34Click(Sender: TObject); //加载电机
begin
  ControlMotor(True); //加载电机
end;

procedure TMainForm.Button2Click(Sender: TObject);  //卸载电机
begin
  ControlMotor(False); //卸载电机
end;

procedure TMainForm.Button22Click(Sender: TObject);
begin
  GetDeviceWorkState; //返回当前工作状态（回调中读取）
end;

procedure TMainForm.Button23Click(Sender: TObject);
begin
  ShowAboutWindow; //关于信息
end;

//基本控制相关==================================================================================================

function CreateRandomNumber(const Len: Word): string; //生成指定长度的随机数字字符串
const
  SourceStr: array[1..62] of Char = (//
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', //
    'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', //
    'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', //
    'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', //
    'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', //
    'o', 'p', 'q', 'r', 's', 't', 'u', 'v', //
    'w', 'x', 'y', 'z', '0', '1', '2', '3', //
    '4', '5', '6', '7', '8', '9');
var
  TmsChar: Char;
begin
  Randomize;

  Result := '';
  repeat
    TmsChar := SourceStr[Random(Length(SourceStr))];

    if ((Length(Result) < 3) and (Ord(TmsChar) < 65)) or ((Length(Result) > 2) and (Ord(TmsChar) > 58)) then //首位不可为0
      Continue;

    Result := Trim(Result + TmsChar);
  until Length(Result) = Len;
end;

procedure TMainForm.Button29Click(Sender: TObject);
begin
  FactoryPassWordEdit.Text := CreateRandomNumber(10);  //6位随机数字作为密码
end;

procedure TMainForm.Button30Click(Sender: TObject);
begin
  WriteFactoryPassWord(//
    PChar(Trim(OLDFactoryPassWordEdit.Text)), //
    PChar(Trim(FactoryPassWordEdit.Text))); //设置厂家密码
end;

procedure TMainForm.Button31Click(Sender: TObject);
begin
  vLanguage := TButton(Sender).Tag;
  siLangDispatcher.Language := siLangDispatcher.LangNames[vLanguage];
end;

procedure TMainForm.CHECKFCPSWBTNClick(Sender: TObject);
begin
  CheckFactoryPassWord(PChar(Trim(TESTFactoryPassWordEdit.Text))); //StrToInt64Def(Trim(FactoryPassWordEdit.Text), 0)); //验证厂家密码
end;

procedure TMainForm.RSTFPSWBTNClick(Sender: TObject);
begin
  ResetFactoryPassWord(PChar('{B1946B01-30FA-4910-9DF0-495D74071D51}')); //初始化厂家密码
end;

procedure TMainForm.Button33Click(Sender: TObject); //清空寄存器输入框
begin
  StorageDataEdit.Clear
end;

//切割操作相关==================================================================================================
procedure TMainForm.LoadSvgFile(const FileName: TFileName);//导入SVG
begin

end;

procedure TMainForm.LoadPltFile(const FileName: TFileName);//导入PLT
var
  vGobjectCount, vInvalidGobjectCount: Integer;
  MinSpeed, Runspeed, MoveSpeed: Integer; //初始速度，运行速度，空移速度，X 轴脉冲长度比，Y 轴脉冲长度比
  MinSpeedPower, RunSpeedPower, CarvePower: Word; //启动、运行速比、切割功率
begin
  if FileExists(FileName) then
  begin
    //赋值
    //======================================================================
    MinSpeed := StrToIntDef(Trim(CutMinSpeedEdit.Text), 20); //初始速度
    Runspeed := StrToIntDef(Trim(CutRunspeedEdit.Text), 40); //运行速度
    MoveSpeed := StrToIntDef(Trim(StorageEdit5.Text), 200); //空移速度
    //
    MinSpeedPower := StrToIntDef(Trim(CutMinSpeedPowerEdit.Text), 10); //启动速比
    RunSpeedPower := StrToIntDef(Trim(CutRunSpeedPowerEdit.Text), 20); //运行速比
    CarvePower := StrToIntDef(Trim(CutPowerEdit.Text), 40); //切割功率
    //======================================================================
    //
    {
      const vFileName: PChar; //数据文件名
      const FinishRun: word; //加工完成后执行的指令(使用32Bit二进制表示设定状态)
      const ZeroPointStyle: Boolean; //加工原点定义：True:机械原点为原点 False:当前坐标点为原点
      const XYZStyle: Byte; //XYZ轴方式：0:XY 1:XZ
      //
      const vMinSpeed, vRunspeed, vMoveSpeed: Double; //初始速度，运行速度，空移速度
      const MinSpeedPower, RunSpeedPower, CarvePower: word; //启动、运行速比、切割功率
      //
      var GobjectCount: Integer; //有效图形对象数量
      var InvalidGobjectCount: Integer //无效图形对象数量
    }
    vPageCount := CreateMicroStepsDataEx(//
      PChar(FileName), //PLT文件名
      BinToDec(IntToStr(FinishRunComtBox.ItemIndex)), //加工完成后执行的指令(使用32Bit二进制表示设定状态) （单选，低位1字节，10进制表示，共256种动作）
      not ZeroPointStyleCheckBox.Checked, //加工原点定义：True:机械原点为原点 False:当前坐标点为原点
      0, //XYZ轴方式：0:XY 1:XZ
      //
      //XStepNum, YStepNum, //X 轴脉冲长度，Y 轴脉冲长度
      //
      MinSpeed, Runspeed, MoveSpeed,  //初始速度，运行速度，空移速度
      MinSpeedPower, RunSpeedPower, CarvePower, //启动、运行速比、切割功率
      //
      vGobjectCount,   //有效图形对象数量
      vInvalidGobjectCount//无效图形对象数量
    ); //读取指定文件名的多线段图形数据，返回数据包总数
    //
    with ProgressBar do
    begin
      Max := vPageCount;
      Position := 0;
      ProgressLabel.Caption := Format(StartWorTip, [0, vPageCount]);
    end;
    //
    if vPageCount > 0 then
    begin
      WorkPanel.Enabled := True;
      StartWorkBtn.Enabled := True; //使能开始
      //
      WorkState := 1;
      //
      ProgressLabel.Caption := Format(//
        '包含 %d 个图形（坐标超限图形 %d 个），实际可加工图形 %d 个', //
        [vGobjectCount, vInvalidGobjectCount, vGobjectCount - vInvalidGobjectCount] //
      );
    end
    else
      ProgressLabel.Caption := '绘图数据错误...';
  end;
end;
//切割操作相关==================================================================================================


//雕刻操作相关==================================================================================================

procedure TMainForm.LoadPicFile(const FileName: TFileName);//导入图像
var
  //======================================================================
  vGrayImage: TFileName;   //灰度图像文件名
  //======================================================================
  vStartX, vStartY: Double; //雕刻起始位置，mm

  //======================================================================
  vImageDPI: Double; //图像分辨率
  vIWidth, vIHeight: Double; //图像宽高，单位mm
  vUnits: Integer; //分辨率单位 0=像素/英寸 1=像素/厘米
  //======================================================================
  vCarvePower: Word; //雕刻功率
  vMinSpeedPower, vRunSpeedPower: Word; //启动、运行速比
  vCarvePixelsX, vCarvePixelsY: Double; //雕刻图像像素数 可返回 (行列点数)
const
  vMod: array[Boolean] of Byte = (0, 1);
  CarveStyle: array[Boolean] of string = ('01', '00'); //00：按行雕刻  01：按列雕刻
begin
  //赋值
  //======================================================================
  vIWidth := StrToFloatDef(Trim(IWidthEdit.Text), 50); //雕刻图像宽度:单位mm
  vIHeight := StrToFloatDef(Trim(IHeightEdit.Text), 50); //雕刻图像高度:单位mm
  //
  vStartX := StrToFloatDef(Trim(IStartXEdit.Text), 0); //起始位置X mm
  vStartY := StrToFloatDef(Trim(IStartYEdit.Text), 0); //起始位置Y mm
  //
  vImageDPI := StrToFloatDef(Trim(GraphicDPIComBox.Text), 300); //图像分辨率
  vUnits := UnitsComboBox.ItemIndex; //分辨率单位0=像素/英寸 1=像素/厘米
  //
  vMinSpeedPower := StrToIntDef(Trim(CarveMinSpeedPowerEdit.Text), 1);
  vRunSpeedPower := StrToIntDef(Trim(CarveRunSpeedPowerEdit.Text), 5); //启动、运行速比
  //
  vCarvePower := StrToIntDef(Trim(CarvePowerEdit.Text), 100); //雕刻功率
  //
  if FileExists(FileName) then
  begin
    vGrayImage := Format('%sTmpFile\%s', [cApplicationPath, ExtractFileName(ChangeFileExt(FileName, '.jpg'))]);
    //
    vPageCount := CreateCarvePictureDataEx(//
      False, //是否清空数据，True为清空，创建多个任务数据需要设置为False，使之成为追加状态
      False, //加工完成后走边框
      PChar(FileName), PChar(vGrayImage), //源文件、目标文件
      False, //加载完数据后是否马上加工
      BinToDec(IntToStr(FinishRunComtBox.ItemIndex)), //加工完成后执行的指令(使用32Bit二进制表示设定状态) （单选，低位1字节，10进制表示，共256种动作）
      False, //加工原点定义：True:机械原点为原点 False:当前坐标点为原点
      0, //XYZ轴方式：0:XY 1:XZ
      //XStepNum, YStepNum, //脉冲长度
      True,  //是否正向雕刻，TRUE为正向雕刻
      0, //0=按行雕刻；1=按列雕刻
      HStep, LStep, //行列间距 1-100
      StrToIntDef(Trim(ErrXEdit.Text), 50), //反向间隙： 0-500
      vMinSpeedPower, vRunSpeedPower, //启动、运行速比
      StrToFloatDef(Trim(XCarveMinSpeedEdit.Text), 30), //雕刻起始速度 mm/s 默认30
      StrToFloatDef(Trim(CarveRunSpeedEdit.Text), 180), //雕刻速度 mm/s 默认180
      vCarvePower, //雕刻功率
      DefaultDPICheckBox.Checked, //是否按原图尺寸输出
      0, //图像旋转方式
      vMod[BWCheckBox.Checked], //图像处理方式 0 灰度图 1 二值图
      vStartX, vStartY, //雕刻起点 mm
      //
      vCarvePixelsX, vCarvePixelsY, //雕刻图像像素数  可返回
      vImageDPI, //图像分辨率 50-300 可返回
      vUnits, //分辨率单位0=像素/英寸 1=像素/厘米  可返回
      vIWidth, vIHeight//打印图像宽高（mm）可返回
    );
  end
  else
    Exit;
  //
  if vPageCount < 1 then
  begin
    ShowMessage(ReturMsg[vPageCount]);
    Exit;
  end;

  begin
    GraphicDPIComBox.Text := Format('%g', [vImageDPI]); //图像分辨率
    UnitsComboBox.ItemIndex := vUnits; //分辨率单位0=像素/英寸 1=像素/厘米
    IWidthEdit.Text := Format('%g', [vIWidth]); //打印图像宽（mm）
    IHeightEdit.Text := Format('%g', [vIHeight]); //打印图像高（mm）
  end;

  //至此已经完成灰度数据的打包处理，准备下发啦
  //-------------------------------------------------------------------------------
  //
  if FileExists(vGrayImage) then
  begin
    GrayImage.Picture.LoadFromFile(vGrayImage);
    //
    with ProgressBar do
    begin
      Max := vPageCount;
      Position := 0;
      ProgressLabel.Caption := Format(StartWorTip, [0, vPageCount]);
    end;
    //
    if vPageCount > 0 then
    begin
      WorkState := 1;
      WorkPanel.Enabled := True;
      StartWorkBtn.Enabled := True; //使能开始雕刻
      //
      {$IFDEF WRITELOG}
      WriteRZ(//
        Format(//
        '当前雕刻的图像共有 %g 行 %g 列，将拆分为 %d 包传输', //
        [vCarvePixelsY, vCarvePixelsX, vPageCount] //
      ), Now);
      WriteRZ(vLine, Now, False);
      {$ENDIF}
    end
    else
    begin
      StartWorkBtn.Enabled := False;
      ProgressLabel.Caption := '雕刻数据太小！';
    end;
  end
  else
  begin
    GrayImage.Picture := nil;
  end;
end;
//雕刻操作相关==================================================================================================

procedure TMainForm.LoadJsonFile(const FileName: TFileName);
begin
  vPageCount := LoadDataFromFile(PChar(FileName));

  //
  with ProgressBar do
  begin
    Max := vPageCount;
    Position := 0;
    ProgressLabel.Caption := Format(StartWorTip, [0, vPageCount]);
  end;
  //
  if vPageCount > 0 then
  begin
    WorkPanel.Enabled := True;
    StartWorkBtn.Enabled := True; //使能开始
    //
    WorkState := 1;
  end;
end;

procedure TMainForm.LaserPWTrackChange(Sender: TObject);
begin
  StorageEdit15.Text := IntToStr(LaserPWTrack.Position);
end;

procedure TMainForm.LoadBinFile(const FileName: TFileName);
begin
  vPageCount := ImportDateFromFile(PChar(FileName));

  //
  with ProgressBar do
  begin
    Max := vPageCount;
    Position := 0;
    ProgressLabel.Caption := Format(StartWorTip, [0, vPageCount]);
  end;
  //
  if vPageCount > 0 then
  begin
    WorkPanel.Enabled := True;
    StartWorkBtn.Enabled := True; //使能开始
    //
    WorkState := 1;
  end;
end;

procedure TMainForm.LoadImageFileBtnClick(Sender: TObject);
var
  vFileName: TFileName;
begin
  TThread.CreateAnonymousThread( //使用多线程
    procedure

      procedure AppendData(const FileName: TFileName);
      begin
        if UpperCase(ExtractFileExt(FileName)) = '.JSON' then
          LoadJsonFile(FileName);
        if UpperCase(ExtractFileExt(FileName)) = '.SVG' then
          LoadSvgFile(FileName);
        if UpperCase(ExtractFileExt(FileName)) = '.BIN' then
          LoadBinFile(FileName);
        if UpperCase(ExtractFileExt(FileName)) = '.PLT' then
          LoadPltFile(FileName);
        if Pos(UpperCase(ExtractFileExt(FileName)), '.BMP.JPG.PNG') > 0 then
          LoadPicFile(FileName);
      end;

    var
      i: Integer;
    begin
      TButton(Sender).Enabled := False;
      case TButton(Sender).Tag of
        0: //导入
          begin
            if DataCheckListBox.Items.Count > 0 then
            begin
              DataCheckListBox.Tag := 0;

              for i := 0 to DataCheckListBox.Items.Count - 1 do
                if DataCheckListBox.Checked[i] then
                begin
                  DataCheckListBox.Tag := 10;
                  vFileName := DataCheckListBox.Items[i];
                  AppendData(vFileName);
                end;
            end;

            if DataCheckListBox.Tag = 0 then
              with PLTOpenDialog do
                if Execute then
                begin
                  AppendData(FileName);
                end;
          end;
        1: //添加
          with PLTOpenDialog do
            if Execute then
            begin
              DataCheckListBox.Items.Append(FileName);
            end;
        2: //删除
          begin
            DataCheckListBox.DeleteSelected;
          end;
        3: //清空
          begin
            DataCheckListBox.Clear;
          end;
        4: //全选
          begin
            for i := 0 to DataCheckListBox.Items.Count - 1 do
              DataCheckListBox.Checked[i] := True;
          end;
        5: //反选
          begin
            for i := 0 to DataCheckListBox.Items.Count - 1 do
              DataCheckListBox.Checked[i] := not DataCheckListBox.Checked[i];
          end;
      end;
      TButton(Sender).Enabled := True;
    end).Start;
end;

//寄存器操作相关==================================================================================================
procedure TMainForm.Button1Click(Sender: TObject);
begin
  StorageAddEdit.Clear
end;

procedure TMainForm.Button43Click(Sender: TObject);
begin
  WriteSysParamToCard( //多点保存到寄存器
    PChar('4,13,14'), //地址 (直接填入地址，以,分隔)
    PChar(Format(//值 (直接填入地址对应顺序的值，以,分隔)
    '%d,%d,%d', [//
    StrToInt64Def(Trim(YCarveMinSpeedEdit.Text), 10000), //雕刻时Y轴起跳速度 1000-200000的范围值 除以1000就是mm单位
    StrToInt64Def(Trim(StorageEdit13.Text), 5), //
    StrToInt64Def(Trim(StorageEdit14.Text), 100)]//
  ))//
  );
end;

procedure TMainForm.Button44Click(Sender: TObject);
begin
  ReadSysParamFromCard('4,13,14'); //多点读取寄存器(直接填入地址，以,分隔)
end;

procedure TMainForm.Button45Click(Sender: TObject);
begin
  if RegisterMainCard(PChar(Trim(ActivationCodeEdit.Text))) <> '' then //注册
    ShowMessage(RegisterMainCardTip);
end;

procedure TMainForm.Button46Click(Sender: TObject);
begin
  GetMainCardRegState
end;

procedure TMainForm.ShowUpdateAskWindow(//发现新版本，弹窗询问是否升级
  const IsFirmwareUpdate: Boolean; //是否固件更新，否为软件更新
  const Flag: string; //标识符
  const vJsonFile: string; //
  const BeforeRunType: Integer; //升级前运行的程序运行方式 0运行并等待结束 1运行后无需等待继续执行升级(默认)
  const BeforeRunExeName: string; //升级开始前运行的程序文件名，为空则升级完不运行
  const AfterRunExeName: string //升级完成后运行的程序文件名，为空则升级完不运行
);
var
  UpdateTips: string; //日志内容
  RetVersion: Integer; //JSON中返回的新版本号
  AJson: TQJson;
begin
  UpdateTips := '';
  {=======================================
  在此解析JSON文件用于升级弹窗显示升级日志
  =======================================}
  if FileExists(vJsonFile) then //无新版本不会生成日志文件，要检测一下
  try
    //解析PLT
    AJson := TQJson.Create;
    try
      AJson.LoadFromFile(vJsonFile);
      UpdateTips := AJson.ForcePath('UpdateLog').AsString; //升级日志
      RetVersion := AJson.ForcePath('ProductVersion').AsInteger; //新版本号
    finally
      FreeAndNil(AJson);
      DeleteFile(vJsonFile); //解析完JSON后，LOG文件即可删除了
    end;
  except
    UpdateTips := '';
  end;

  if MessageBox(Handle, //
    PChar(Format(//
    FoundNewVersion, //
    [UPD[IsFirmwareUpdate], RetVersion, UpdateTips])), //
    UpdateTip, MB_YESNO + MB_ICONQUESTION) = IDYES then
  begin
    StartVersionUpdate(//更新软件版本
      IsFirmwareUpdate, //是否固件更新，否为软件更新
      PChar(cFlag[UpdaeRadioGroup.ItemIndex]), //标识符
      '', //本地文件
      0, //升级为最新版
      BeforeRunType, //升级前运行的程序运行方式 0运行并等待结束 1运行后无需等待继续执行升级
      PChar(BeforeRunExeName), //升级前要运行的程序，支持.BAT .EXE
      PChar(AfterRunExeName) //升级完要启动的EXE
    );
  end;
end;

procedure TMainForm.Button64Click(Sender: TObject);
const
  cFilter: array[Boolean] of string = (//
    '软件升级包(*.tar)|*.tar', //
    '固件升级包(*.bin;*.cab)|*.bin;*.cab' //
    );
begin
  {
    function StartVersionUpdate(//更新软件
      const IsFirmwareUpdate: Boolean; //是否固件更新，否为软件更新
      const Flag: PChar; //标识符
      const LocalFile: PChar; //本地升级文件（网络升级时无效）
      const UpdateVersion: Integer; //要升级的版本号 指定格式为201201则升级此版本，值为0则升级到最新版
      const BeforeRunType: Integer; //升级前运行的程序运行方式 0运行并等待结束 1运行后无需等待继续执行升级(默认)
      const BeforeRunExeName: PChar; //升级开始前运行的程序文件名，为空则升级完不运行
      const AfterRunExeName: PChar //升级完成后运行的程序文件名，为空则升级完不运行
    ): Integer; stdcall; external DllFile;
  }
  IsFirmwareUpdate := TButton(Sender).Tag = 1; //1为固件

  case UpdaeRadioGroup.ItemIndex of
    0: //本地固件
      with BinOpenDialog do
      begin
        Filter := cFilter[IsFirmwareUpdate];
        if Execute() then
        begin
          StartVersionUpdate(//更新软件版本
            IsFirmwareUpdate, //是否固件更新，否为软件更新
            PChar(cFlag[UpdaeRadioGroup.ItemIndex]), //标识符
            PChar(FileName), //本地升级文件
            0, //要升级的版本号 指定格式为201201则升级此版本，值为0则升级到最新版
            0, //升级前运行的程序运行方式 0运行并等待结束 1运行后无需等待继续执行升级(默认)
            PChar(BeforeRunExeName), //升级前要运行的程序，支持.BAT .EXE
            PChar(AfterRunExeName) //升级完要启动的EXE
          );
        end;
      end;
    1, 2: //网络固件
      begin
        {
          procedure CheckVersionUpdate(//查询是否有新版本
            const IsFirmwareUpdate: Boolean; //是否固件更新，否为软件更新
            const Flag: PChar; //标识符
            const CurrentVersion: Integer; //当前版本号
            const VersionNoteToJsonFile: string //Json格式的更新内容文件名，值为空表示不返回也不生成文件
          ); stdcall; external DllFile;//通过回调返回有无新版本情况
        }
        case IsFirmwareUpdate of
          False: //软件升级
            CurrentVersion := API_GetVertionNumber(CompileDT); //软件版本
          True: //固件升级
            CurrentVersion := StrToIntDef(GetMainHardVersion, 201201); //固件版本
        end;
        //ShowMessage('JsonFile = ' + JsonFile);
        CheckVersionUpdate(//
          IsFirmwareUpdate, //
          PChar(cFlag[UpdaeRadioGroup.ItemIndex]), //标识符
          CurrentVersion, //当前版本号
          PChar(JsonFile)//Json格式的更新内容文件名，值为空表示不返回也不生成文件
        );
      end
  end;
end;

procedure TMainForm.Button66Click(Sender: TObject);
begin
  SetLanguge(TButton(Sender).Tag); //使用语言切换
end;

procedure TMainForm.Button68Click(Sender: TObject);
var
  MinSpeed, Runspeed, MoveSpeed: Integer; //初始速度，运行速度，空移速度
  MinSpeedPower, RunSpeedPower, CarvePower: Word; //启动、运行速比、切割功率
begin
  //赋值
  //======================================================================
  MinSpeed := StrToIntDef(Trim(CutMinSpeedEdit.Text), 20); //初始速度
  Runspeed := StrToIntDef(Trim(CutRunspeedEdit.Text), 40); //运行速度
  MoveSpeed := StrToIntDef(Trim(StorageEdit5.Text), 200); //空移速度
  //
  MinSpeedPower := StrToIntDef(Trim(CutMinSpeedPowerEdit.Text), 10); //启动速比
  RunSpeedPower := StrToIntDef(Trim(CutRunSpeedPowerEdit.Text), 20); //运行速比
  CarvePower := StrToIntDef(Trim(CutPowerEdit.Text), 40); //切割功率
  //======================================================================
  {
    procedure LaserDrawRectangle(//
      const XYZStyle: Byte; //XYZ轴方式：0:XY 1:XZ
      const ZeroPointStyle: Boolean; //加工原点定义：True:机械原点为原点 False:当前坐标点为原点
      const LaserOn: Boolean; //是否开激光
      const Left, Top, Right, Bottom: Integer; //矩形区坐标
      const vMinSpeed, vRunspeed, vMoveSpeed: Double; //初始速度，运行速度，空移速度
      const CarvePower, MinSpeedPower, RunSpeedPower: Word //切割功率启动、运行速比
    ); stdcall;
  }
  LaserDrawRectangle(//
    0, //
    True, //
    True, //
    20, 20, 120, 120, //
    MinSpeed, Runspeed, MoveSpeed, //
    CarvePower, MinSpeedPower, RunSpeedPower//
  )
end;

procedure TMainForm.Button48Click(Sender: TObject);
begin
  ShowMessage(Format('%s', [GetHardwareKeyID]));
end;

procedure TMainForm.Button49Click(Sender: TObject);
begin
  GetMainCardInfo; //查询当前联机的板卡及绑定机器更多详细数据
end;

procedure TMainForm.Button4Click(Sender: TObject);
begin
  ReadSysParamFromCard(PChar(StorageAddEdit.Text)); //读取寄存器
end;

procedure TMainForm.Button5Click(Sender: TObject);
begin
  WriteSysParamToCard(//多点保存到寄存器
    PChar(Trim(StorageAddEdit.Text)), PChar(Trim(StorageDataEdit.Text)));
end;

procedure TMainForm.Button35Click(Sender: TObject);
begin
  if MessageBox(Handle, ResoreTip, Titel, MB_YESNO + MB_ICONQUESTION + MB_DEFBUTTON2 + MB_TOPMOST) = IDYES then
  try
    StorageEdit3.Text := '15'; //
    StorageEdit7.Text := '120'; //
    StorageEdit8.ItemIndex := 3; //
    StorageEdit9.Text := '6.329114'; //
    StorageEdit10.Text := '6.329114'; //
    StorageEdit21.Text := '2'; //
    StorageEdit22.Text := '0'; //
    MaxWorkSpaceSizeXEdit.Text := '320'; //
    MaxWorkSpaceSizeYEdit.Text := '210'; //
    PrinterDrawUnitEdit.Text := '1016'; //绘图单位
  finally
    SaveParmBtnClick(nil); //保存到寄存器
  end;
end;

procedure TMainForm.Button36Click(Sender: TObject);
begin
  ReadSysParamFromCard(PChar('3,7,8,9,10,21,22,38,39')); //多点读取寄存器(直接填入地址，以,分隔)
end;

procedure TMainForm.Button37Click(Sender: TObject);
begin
  WriteSysParamToCard( //多点保存到寄存器
    PChar('3,7,8,9,10,21,22,38,39'), //地址 (直接填入地址，以,分隔)
    PChar(//
    Format(//值 (直接填入地址对应顺序的值，以,分隔)
    '%g,%g,%d,%g,%g,%g,%g,%d,%d', [//
    StrToFloatDef(Trim(StorageEdit3.Text), 5), //
    StrToFloatDef(Trim(StorageEdit7.Text), 100), //
    StorageEdit8.ItemIndex + 1, //工作象限
    StrToFloatDef(Trim(StorageEdit9.Text), 6.329114), //
    StrToFloatDef(Trim(StorageEdit10.Text), 6.329114), //
    StrToFloatDef(Trim(StorageEdit21.Text), 0), //
    StrToFloatDef(Trim(StorageEdit22.Text), 0), //
    StrToIntDef(Trim(MaxWorkSpaceSizeXEdit.Text), 320) * 256 * 256 + //高位
    StrToIntDef(Trim(MaxWorkSpaceSizeYEdit.Text), 210), //低位
    StrToIntDef(Trim(PrinterDrawUnitEdit.Text), 1016) //绘图单位
    ]//
  )//
  ));
end;

procedure TMainForm.Button26Click(Sender: TObject);
begin
  WriteSysParamToCard( //多点保存到寄存器
    PChar('31,32,33,34,35,36'), //地址 (直接填入地址，以,分隔)   4294.967295
    PChar(//
    Format(//值 (直接填入地址对应顺序的值，以,分隔)
    '%g,%g,%g,%g,%g,%g', [//
    StrToFloatDef(Trim(XMoveToPostEdit1.Text), 0), //
    StrToFloatDef(Trim(YMoveToPostEdit1.Text), 0), //
    StrToFloatDef(Trim(XMoveToPostEdit2.Text), 0), //
    StrToFloatDef(Trim(YMoveToPostEdit2.Text), 0), //
    StrToFloatDef(Trim(XMoveToPostEdit3.Text), 0), //
    StrToFloatDef(Trim(YMoveToPostEdit3.Text), 0)//
    ]//
  )//
  ));
end;

procedure TMainForm.Button28Click(Sender: TObject);
begin
  ReadSysParamFromCard(PChar('31,32,33,34,35,36')); //多点读取寄存器(直接填入地址，以,分隔)
end;

procedure TMainForm.ReLoadParmBtnClick(Sender: TObject); //读取参数
begin
  ReadSysParamFromCard(PChar('5,11,12,23,24,25,27,40')); //多点读取寄存器(直接填入地址，以,分隔)
end;

procedure TMainForm.ResetFacPswBTNClick(Sender: TObject);
begin
  RSTFPSWBTN.Click;
  CHECKFCPSWBTN.Click;
end;

procedure TMainForm.Button19Click(Sender: TObject);
begin
  ReadSysParamFromCard('15,16,17'); //多点读取寄存器(直接填入地址，以,分隔)
end;

procedure TMainForm.Button60Click(Sender: TObject);
begin
  CardIDEdit.SelectAll;
  CardIDEdit.CopyToClipboard;
  ShowMessage(CopyToClipboardTip);
end;

procedure TMainForm.Button6Click(Sender: TObject);
begin
  ReadSysParamFromCard(PChar('18,19,20')); //多点读取寄存器(直接填入地址，以,分隔)
end;

procedure TMainForm.Button11Click(Sender: TObject);
begin
  WriteSysParamToCard( //多点保存到寄存器
    PChar('18,19,20'), //地址 (直接填入地址，以,分隔)
    PChar(Format(//值 (直接填入地址对应顺序的值，以,分隔)
    '%d,%d,%d', [//
    StrToInt64Def(Trim(StorageEdit18.Text), 5), //
    StrToInt64Def(Trim(StorageEdit19.Text), 100), //
    StrToInt64Def(Trim(StorageEdit20.Text), 100) //
    ]//
  ))//
  );
end;

procedure TMainForm.Button50Click(Sender: TObject);
begin
  GetDeviceID(False); //获取本机唯一ID(回调中返回)
end;

procedure TMainForm.Button51Click(Sender: TObject);
begin
  GetClientAddr(False); //获取本机的外网IP地址(回调中返回)
end;

procedure TMainForm.Button52Click(Sender: TObject);
begin
  WriteSysParamToCard( //多点保存到寄存器
    PChar('15,16,17'), //地址 (直接填入地址，以,分隔)
    PChar(Format(//值 (直接填入地址对应顺序的值，以,分隔)
    '%d,%d,%d', [//
    StrToInt64Def(Trim(StorageEdit15.Text), 100), //
    StrToInt64Def(Trim(StorageEdit16.Text), 1), //
    StrToInt64Def(Trim(StorageEdit17.Text), 1) //
    ]//
  ))//
  );
end;

procedure TMainForm.Button53Click(Sender: TObject);
begin
  SaveMainBoardParamsToServer  //参数保存至服务器
end;

procedure TMainForm.Button54Click(Sender: TObject);
begin
  ReadMainBoardParamsFromServer; //从服务器读取用户板卡配置参数
end;

procedure TMainForm.Button55Click(Sender: TObject);
begin
//读取加密锁信息
//返回：经销商;厂址;区域代码;售后电话;QQ号码;微信号码;电子邮箱;网址;ApplicationID
  ShowMessage(GetHardwareKeyInfo); //
end;

procedure TMainForm.Button56Click(Sender: TObject);
begin
  LaserTubeZeroClearing(PChar(ResetPassWordEdit.Text)); //激光管使用时间清零
end;

procedure TMainForm.Button57Click(Sender: TObject);
begin
  if ExportSFileFromCDR('D:\Demo.SVG') = 0 then
    ShowMessage('导出SVG文件成功！')
  else
    ShowMessage('导出SVG文件失败！')
end;

procedure TMainForm.Button58Click(Sender: TObject);
var
  FName: string;
begin
  FName := 'D:\Demo.PLT';
  if ExportSFileFromCDR(PChar(FName)) = 0 then
  try
    LoadPltFile(FName);
  finally
    DeleteFile(FName);
    StartWorkBtnClick(nil);
  end
  else
    ShowMessage('切割失败！')
end;

procedure TMainForm.Button59Click(Sender: TObject);
var
  FName: string;
begin
  FName := 'D:\Demo.BMP';
  if ExportSFileFromCDR(PChar(FName)) = 0 then
  try
    LoadPicFile(FName);
  finally
    DeleteFile(FName);
    StartWorkBtnClick(nil);
  end
  else
    ShowMessage('雕刻失败！')
end;

procedure TMainForm.SaveParmBtnClick(Sender: TObject); //保存参数
begin
  WriteSysParamToCard( //多点保存到寄存器
    PChar('5,11,12,23,24,25,27,40'), //地址 (直接填入地址，以,分隔)
    PChar(//
    Format(//值 (直接填入地址对应顺序的值，以,分隔)
    '%g,%g,%g,%d,%d,%d,%d,%g', [//
    StrToFloatDef(Trim(StorageEdit5.Text), 100), //
    StrToFloatDef(Trim(StorageEdit11.Text), 0), //
    StrToFloatDef(Trim(StorageEdit12.Text), 0), //
    StrToInt64Def(Trim(StorageEdit23.Text), 0), //
    StrToInt64Def(Trim(StorageEdit24.Text), 0), //
    StrToInt64Def(Trim(StorageEdit25.Text), 1000), //
    StrToInt64Def(Trim(StorageEdit27.Text), 1000), //
    StrToFloatDef(Trim(StorageEdit40.Text), 200)]//
  )//
  ));
end;

procedure TMainForm.DefaultParmBtnClick(Sender: TObject);
begin
  if MessageBox(Handle, RestoreParamTip, Titel, MB_YESNO + MB_ICONQUESTION + MB_DEFBUTTON2 + MB_TOPMOST) = IDYES then
  try
    StorageEdit5.Text := '200'; //
    StorageEdit11.Text := '0'; //
    StorageEdit12.Text := '0'; //
    StorageEdit23.Text := '0'; //
    StorageEdit24.Text := '0'; //
    StorageEdit25.Text := '1000'; //
    StorageEdit27.Text := '1000'; //
    StorageEdit40.Text := '200'; //
  finally
    SaveParmBtnClick(nil); //保存到寄存器
  end;
end;
//寄存器操作相关==================================================================================================

//系统操作相关==================================================================================================

procedure TMainForm.ComButtonClick(Sender: TObject); //联机 脱机
begin
  case TButton(Sender).Tag of
    0:
      begin
        InitComPort(GetComPortName(ComPortBox.Text)); //1表示com1，类推
      end
  else
    begin
      UnInitComPort;
    end;
  end;
end;

procedure TMainForm.Button7Click(Sender: TObject); //设置最大加工幅面
begin
  SetSoftwareInitialization(//设置最大加工幅面
    StrToIntDef(Trim(PrinterDrawUnitEdit.Text), 1016), //
    0, 0, //
    StrToIntDef(Trim(MaxWorkSpaceSizeXEdit.Text), 297), //
    StrToIntDef(Trim(MaxWorkSpaceSizeYEdit.Text), 210)//
  );
end;

procedure TMainForm.Button8Click(Sender: TObject);
begin
  CalibrateGuideExecute; //输出校准向导
end;

//系统操作相关==================================================================================================



//快速移动相关==================================================================================================

procedure TMainForm.StorageEdit5Change(Sender: TObject);  //快速移动速度
begin
  MoveWorkSpeed := StrToIntDef(Trim(StorageEdit5.Text), 100);
end;

procedure TMainForm.TrackBar1Change(Sender: TObject);
begin
  StorageEdit16.Text := IntToStr(TrackBar1.Position);
end;

//XYZ轴方式：0:XY 1:XZ ZeroPointStyle 加工原点定义：True:机械原点为原点 False:当前坐标点为原点
procedure RunMove(const XYZ: Boolean; const ZeroPointStyle: Boolean; mTag: Integer; mXStep, mYStep: Double);//移动到指定位置 XYZ = True：快速移动至XZ  False：快速移动至XY
begin
  try
    case mTag of
      1: //左上
        begin
          MoveToX := MoveToX - mXStep;
          MoveToY := MoveToY + mYStep;
        end;
      2: //上
        begin
          MoveToY := MoveToY + mYStep;
        end;
      3: //右上
        begin
          MoveToX := MoveToX + mXStep;
          MoveToY := MoveToY + mYStep;
        end;
      4: //左
        begin
          MoveToX := MoveToX - mXStep;
        end;
      5: //回原点
        begin

        end;
      6: //右
        begin
          MoveToX := MoveToX + mXStep;
        end;
      7: //左下
        begin
          MoveToX := MoveToX - mXStep;
          MoveToY := MoveToY - mYStep;
        end;
      8: //下
        begin
          MoveToY := MoveToY - mYStep;
        end;
      9: //右下
        begin
          MoveToX := MoveToX + mXStep;
          MoveToY := MoveToY - mYStep;
        end;
    end;
    //
    {
      WorkArea: Byte = 1; //工作象限
      WorkSpaceWidth, WorkSpaceHeight: Word; //加工幅面宽高
    }
    //MainForm.Caption := Format('MoveToX = %g WorkSpaceWidth = %d MoveToY = %g WorkSpaceHeight = %d', [MoveToX, WorkSpaceWidth, MoveToY, WorkSpaceHeight]);
    case WorkArea of
      1: //
        begin
          if MoveToX < 0 then
            MoveToX := 0
          else if MoveToX > WorkSpaceWidth then
            MoveToX := WorkSpaceWidth;

          if MoveToY < 0 then
            MoveToY := 0
          else if MoveToY > WorkSpaceHeight then
            MoveToY := WorkSpaceHeight;
        end;
      2: //
        begin
          if MoveToX > 0 then
            MoveToX := 0
          else if MoveToX < -WorkSpaceWidth then
            MoveToX := -WorkSpaceWidth;

          if MoveToY < 0 then
            MoveToY := 0
          else if MoveToY > WorkSpaceHeight then
            MoveToY := WorkSpaceHeight;
        end;
      3: //
        begin
          if MoveToX > 0 then
            MoveToX := 0
          else if MoveToX < -WorkSpaceWidth then
            MoveToX := -WorkSpaceWidth;

          if MoveToY > 0 then
            MoveToY := 0
          else if MoveToY < -WorkSpaceHeight then
            MoveToY := -WorkSpaceHeight;
        end;
      4: //
        begin
          if MoveToX < 0 then
            MoveToX := 0
          else if MoveToX > WorkSpaceWidth then
            MoveToX := WorkSpaceWidth;

          if MoveToY > 0 then
            MoveToY := 0
          else if MoveToY < -WorkSpaceHeight then
            MoveToY := -WorkSpaceHeight;
        end;
    end;

    begin
      MainForm.PosLabel.Caption := Format('X = %g Y = %g Z = %g', [MoveToX, MoveToY, MoveToZ]);
      //加工原点定义：True:机械原点为原点 False:当前坐标点为原点 目标点坐标为有符号整数
      LPenQuickMoveTo(0, True, (MoveToX), (MoveToY), 0, MoveMinSpeed, MoveWorkSpeed); //快速移动到指定坐标点
    end
  except
  end;
end;

procedure MoveZ(const mTag: Integer; mZStep: Double);
begin
  case mTag of
    2: //上
      begin
        MoveToZ := MoveToZ + mZStep;
      end;
    8: //下
      begin
        MoveToZ := MoveToZ - mZStep;
      end;
  end;
  if MoveToZ > 0 then
    MoveToZ := 0;

  MainForm.PosLabel.Caption := Format('X = %g Y = %g Z = %g', [MoveToX, MoveToY, MoveToZ]);
  //XYZ轴方式：0:XY 1:XZ 加工原点定义：True:机械原点为原点 False:当前坐标点为原点 目标点坐标为有符号整数
  LPenQuickMoveTo(1, True, (MoveToX), (MoveToZ), MoveToZ, MoveMinSpeed, MoveWorkSpeed); //快速移动到指定坐标点
end;

procedure TMainForm.Button61MouseDown(Sender: TObject; Button: TMouseButton; Shift: TShiftState; X, Y: Integer); //Z上下移
begin
  MoveZ(TButton(Sender).Tag, StrToFloatDef(Trim(ZMoveStepEdit.Text), 1.0));
end;

procedure TMainForm.Button12MouseDown(Sender: TObject; Button: TMouseButton; Shift: TShiftState; X, Y: Integer);
begin
  MoveTimer.Tag := TButton(Sender).Tag;
  RunMove(False, True, MoveTimer.Tag, StrToFloatDef(Trim(XMoveStepEdit.Text), 1.0), StrToFloatDef(Trim(YMoveStepEdit.Text), 1.0)); //移动到指定位置
  CheckMouseTimer.Enabled := True; //按下鼠标启动延迟检测
end;

procedure TMainForm.Button12MouseUp(Sender: TObject; Button: TMouseButton; Shift: TShiftState; X, Y: Integer);
begin   //弹起鼠标停止检测
  MoveTimer.Enabled := False;
  CheckMouseTimer.Enabled := False;
end;

procedure QuickMoveToPos(const X, Y: string); //快速移动至
var
  MoveToX, MoveToY: Double;
{
  cWorkAreaX: array[1..4] of Integer = (1, -1, -1, 1); //1-4象限X符号
  cWorkAreaY: array[1..4] of Integer = (1, 1, -1, -1); //1-4象限Y符号
}
begin
  MoveToX := StrToFloatDef(X, 50) * cWorkAreaX[WorkArea]; //
  MoveToY := StrToFloatDef(Y, 100) * cWorkAreaY[WorkArea]; //
  //加工原点定义：True:机械原点为原点 False:当前坐标点为原点
  LPenQuickMoveTo(0, not MainForm.ZeroPointStyleCheckBox.Checked, (MoveToX), (MoveToY), 0, MoveMinSpeed, MoveWorkSpeed);  //XY False XZ True
end;

procedure TMainForm.Button3Click(Sender: TObject);  //快速移动至
begin
  case TButton(Sender).Tag of
    0:
      QuickMoveToPos(Trim(XMoveToPostEdit.Text), Trim(YMoveToPostEdit.Text)); //快速移动至
    1:
      QuickMoveToPos(Trim(XMoveToPostEdit1.Text), Trim(YMoveToPostEdit1.Text)); //快速移动至
    2:
      QuickMoveToPos(Trim(XMoveToPostEdit2.Text), Trim(YMoveToPostEdit2.Text)); //快速移动至
    3:
      QuickMoveToPos(Trim(XMoveToPostEdit3.Text), Trim(YMoveToPostEdit3.Text)); //快速移动至
  end;
end;

procedure TMainForm.MoveIntervalEditChange(Sender: TObject); //调整移动频率
var
  mInteval: Integer;
begin
  mInteval := 300;
  try
    mInteval := StrToIntDef(Trim(MoveIntervalEdit.Text), 300);
    if (mInteval < 100) or (mInteval > 3000) then
      mInteval := 300;
  except
  end;
  MoveTimer.Interval := mInteval;
end;

procedure TMainForm.MoveTimerTimer(Sender: TObject);
begin
  RunMove(True, not ZeroPointStyleCheckBox.Checked, MoveTimer.Tag, StrToIntDef(Trim(XMoveStepEdit.Text), 1), StrToIntDef(Trim(YMoveStepEdit.Text), 1)); //移动到指定位置
end;

procedure TMainForm.CardIDtoServerBTNClick(Sender: TObject);
begin
  //Demo中：
  //有板卡联机成功，检测是否注册
  //未注册，入库按钮不可用
  //已注册，入库按钮可用
  //点击“入库”，把当前板卡类型、ID和注册码提交到后台EXE
  //等待后台处理反馈（入库成功或失败）
  {$IFDEF SUPPERADMIN}
  if (Trim(CardIDEdit.Text) <> '') and (Trim(ActivationCodeEdit.Text) <> '') then
    ReportCardInfo('MBoardInfo', Format(//
      '%s;%s;%s', //
      [GetMainBoardStyle, Trim(CardIDEdit.Text), Trim(ActivationCodeEdit.Text)]//
    ));
  {$ENDIF}
end;

procedure TMainForm.CheckMouseTimerTimer(Sender: TObject);
begin
  MoveTimer.Enabled := True; //启动自动移动指令发送
end;

//快速移动相关==================================================================================================

procedure TMainForm.OLDFactoryPassWordEditKeyPress(Sender: TObject; var Key: Char);
begin
  if not (Key in ['0'..'9', 'A'..'Z', 'a'..'z', #8, #13]) then
    Key := #0;
end;

procedure TMainForm.CutMinSpeedEditKeyPress(Sender: TObject; var Key: Char);
begin
  if not (Key in ['-', '0'..'9', 'A'..'Z', 'a'..'z', #8, #13]) then
    Key := #0;
end;

procedure TMainForm.CutPWTrackChange(Sender: TObject);
begin
  StorageEdit18.Text := IntToStr(CutPWTrack.Position);
end;

end.

