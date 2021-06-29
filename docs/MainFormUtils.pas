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
    function LoadSysComList(const Com: string = ''): Integer; //�г�COM��
    //
    procedure vProgress(const vPostion: Integer; const vTotalCount: Integer); cdecl;//�������ص�
    procedure vSysMessage(const SysMsgIndex: Integer; const SysMsgCode: Integer; const SysEventData: PChar); cdecl;//��Ϣ�ص�
    //
    procedure LoadJsonFile(const FileName: TFileName); //����Json
    procedure LoadBinFile(const FileName: TFileName); //����Bin
    procedure LoadSvgFile(const FileName: TFileName); //����SVG
    procedure LoadPltFile(const FileName: TFileName); //����PLT
    procedure LoadPicFile(const FileName: TFileName); //����ͼ��
    //
    procedure ShowUpdateAskWindow(//�����°汾������ѯ���Ƿ�����
      const IsFirmwareUpdate: Boolean; //�Ƿ�̼����£���Ϊ�������
      const Flag: string; //��ʶ��
      const vJsonFile: string; //
      const BeforeRunType: Integer; //����ǰ���еĳ������з�ʽ 0���в��ȴ����� 1���к�����ȴ�����ִ������(Ĭ��)
      const BeforeRunExeName: string; //������ʼǰ���еĳ����ļ�����Ϊ���������겻����
      const AfterRunExeName: string //������ɺ����еĳ����ļ�����Ϊ���������겻����
    );
  public
    { Public declarations }
  end;

var
  MainForm: TMainForm;
//=====================================================================================================
  vLanguage: Integer = 0;
  cApplicationPath: string = ''; //APP����·��

  vAutoWorkCount: Integer = 0;
  ProcStr: string = '';
  CurrentVersion: Integer = 0; //��ǰ�汾��

  IsFirmwareUpdate: Boolean; //�Ƿ�̼����£���Ϊ�������
  BeforeRunExeName: string; //����ǰ���еĳ���
  AfterRunExeName: string; //���������еĳ���
  JsonFile: string; //������־�ļ���
  //**********************************************
  XStepNum: Double; //Ϊ X �����峤�� һ�������ƶ��ľ��� ��λmm
  YStepNum: Double; //Ϊ Y �����峤�� һ�������ƶ��ľ��� ��λmm

  RetOriginalSpeed: Double = 200; //��ԭ���ٶ�
  HStep: Word; //�м��
  LStep: Word; //�м��:���ؼ��
  //**********************************************
  vPageCount: Integer = 0; //������
  //
  MinSpeed: Word = 20; //Ϊʵ���趨��ʼ�ٶ� �����������ʱ����ʼ�ٶȣ����ٶ�ֵ��С�������������������󽫵��µ����������
  Runspeed: Word = 100; //Ϊʵ���趨�����ٶ� �����ɼ��ٹ��̺�Ĺ����ٶ�
  EndSpeed: Word = 20; //Ϊʵ���趨����ٶ� ���һ���߶�ʱ��������ٶȣ���������λ��ǰ�������ݸ��ٶ���ǰ����
  //
  MoveMinSpeed: Integer = 200; //�����ƶ������ٶ�
  MoveWorkSpeed: Integer = 200; //Ϊʵ���趨�Ŀ����ٶ�: Word; //�����ƶ��ٶ�

  MoveToX, MoveToY, MoveToZ: Double; //��ǰ����...��λmm
  WorkArea: Byte = 1; //��������
  WorkSpaceWidth, WorkSpaceHeight: Word; //�ӹ�������
  //
  WorkState: Integer = 2; //����״̬
  IsRequestAndContinueDLGshow: Boolean = False; //����ϵ�������DLG�Ƿ��Ѿ���ʾ
  //
  XMoveToPos1: Double = 0;
  YMoveToPos1: Double = 0; //�� 1 ��ӹ�ԭ��
  XMoveToPos2: Double = 0;
  YMoveToPos2: Double = 0; //�� 2 ��ӹ�ԭ��
  XMoveToPos3: Double = 0;
  YMoveToPos3: Double = 0; //�� 3 ��ӹ�ԭ��
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
  CardAciveDateTip = 'Board activation date��%s';
  MachineRegDateTip = 'Machine registration date��%s';
  MachineAciveDateTip = 'Machine activation date��%s';
  MachineMaintenanceTimesTip = 'Machine maintenance times��%s';
  InvalidMachineIDTip = 'The machine ID entered during activation is invalid...';
{$ELSE}
  cCaption: array[0..2] of string = ('��ͣ', '����', '��ͣ');
  cLaserStr: array[5..6] of string = ('�ؼ���', '������');
  CapStr: array[0..1] of string = ('����', '�ѻ�');
  ReturMsg: array[-4..0] of string = ('���ݷְ�������', '����ת������', '����������', 'ת��ʧ��', 'ת��ʧ��');
  //
  Titel = '��ʾ';
  InsertUsbDog = '������������';
  OutSize = '�������ɼӹ����棡';
  AskContinue = '��ǰ��δ�ӹ���ɵ�����Ҫ�����ӹ���';
  RegisterOK = '�忨ע��ɹ�';
  RegisterError = '�忨ע��ʧ��';
  StartWorTip = '%d ���ѷ� / �� %d ��';
  StartUpdateTip = '%d Kb / �� %d Kb';
  DownLoadFileCountsTip = '���� %s ���ļ���Ҫ����...';
  DownLoadFileIndexTip = '׼�����µ� %s ���ļ� >> %s';
  FactroyPasswordInValidTip = '�����������';
  ChangeFactroyPasswordOKTip = '���������޸ĳɹ�';
  ChangeFactroyPasswordErrorTip = '���������޸�ʧ��';
  MainCardIDErrorTip = '�忨 ID ��Ч��';
  EnLockerIDErrorTIP = '������ ID ��Ч��';
  IsLatestVersionTip = '��ǰ�汾�Ѿ�����';
  SoftUpdateFailedTip = '�������ʧ��';
  UpdateFirmwareEndTip = '�忨�̼������ɹ���';
  TimeConsumingTip = '���μӹ������ܼƺ�ʱ:%s';
  WorkAreaTip = '��ǰ��������Ϊ �� %d ����';
  RegisterMainCardTip = 'ע����ɺ�����忨�����ϵ磡';
  UPD: array[Boolean] of string = ('���', '�̼�');
  FoundNewVersion= '%s�����°汾�Ƴ����汾��:%d' + #13#10#13#10 + '%s' + #13#10#13#10 + '�Ƿ��������£�';
  UpdateTip='��������';
  ResoreTip ='Ҫ�Ѳ����ָ�ΪĬ��ֵ��';
  CopyToClipboardTip = '�忨ID�Ѿ����Ƶ������棡';
  RestoreParamTip = 'Ҫ�����������ָ�ΪĬ��ֵ��';
  CardActivedTip= '�Ѽ��';
  CardRegDateTip='�忨ע�����ڣ�%s';
  CardAciveDateTip ='�忨�������ڣ�%s';
  MachineRegDateTip ='����ע�����ڣ�%s';
  MachineAciveDateTip ='�����������ڣ�%s';
  MachineMaintenanceTimesTip ='����ά��������%s';
  InvalidMachineIDTip ='����ʱ����Ļ���ID��Ч...';
{$ENDIF}

  vLine = '-------------------------------------------------------------------------------------';
  cOpen: array[Boolean] of TColor = (clBlack, clRed); //����״̬
  cWorkAreaX: array[1..4] of Integer = (1, -1, -1, 1); //1-4����X����
  cWorkAreaY: array[1..4] of Integer = (1, 1, -1, -1); //1-4����Y����

  cFlag: array[0..2] of string = (//
    '{7ADDA8F6-69A6-410A-B17F-260733FA50D5}', //���ع̼�
    '{4A5F9C85-8735-414D-BCA7-E9DD111B23A8}', //�ڲ�����
    '{CC9094BA-2991-4CE2-A514-BF0EA3685D05}'//�����
    );
//=====================================================================================================

{$INCLUDE CompileInfo.inc}
{$DEFINE WRITELOG} //������־���>��Ҫ

implementation

uses
  LaserLibUtils, System.StrUtils, Winapi.ShellAPI, UpdateFirmwareFormUtils,
  ActiveMBFormUtils, qjson, System.IOUtils, Cromis.Client, Simple.Loger;
{$R *.dfm}

function BinToDec(Value: string): Integer; //������ת��Ϊʮ����
var
  str: string;
  i: Integer;
begin
  str := UpperCase(Value);
  Result := 0;
  for i := 1 to Length(str) do
    Result := Result * 2 + Ord(str[i]) - 48;
end;

//��־���==================================================================================================

procedure TMainForm.N1Click(Sender: TObject); //�����־
begin
  ClearLog; //�����־
end;

procedure TMainForm.N3Click(Sender: TObject);  //������־
begin
  SaveLogToFile; //������־
end;

//��־���==================================================================================================

//COM�������==================================================================================================

function GetComPortName(const ComStr: string): Word; //��ȡCOM���
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

function TMainForm.LoadSysComList(const Com: string = ''): Integer;  //��ȡCOM�б��Զ����ӵ�һ��
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
      ComStr := GetComPortList; //����COM�б�

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

        InitComPort(GetComPortName(ComPortBox.Text)); //1��ʾcom1������
      end
    end;
  end;
end;

//COM�������==================================================================================================

//ϵͳ�ص����==================================================================================================

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
  //����״̬��ִ�е�ָ�X���꣬Y���꣬Z���꣬���⹦�ʣ��Ҷ�ֵ
  iWorkState, iCmdCode, iLaserPower, iGray: Integer;
  iXPos, iYPos, iZPos: Double;
  //
  vUserName: PChar; //����
  vAddress: PChar; //��ַ
  vTelphone: PChar; //�绰
  vQQ: PChar; //QQ
  vWX: PChar; //WX
  vEmail: PChar; //����
  Country: PChar; //����
  Distributor: PChar; //������
  Trademark: PChar; //Ʒ��
  Model: PChar; //�ͺ�
  MachineID: PChar; //����ID
begin
//--------------------------------------------------------------------------------------------------------------------------
  WriteRZ(Format(' SysMsgIndex = %d SysMsgCode = %d SysEventData = %s', [SysMsgIndex, SysMsgCode, SysEventData]), Now); //
  WriteRZ(vLine, Now, False);
//--------------------------------------------------------------------------------------------------------------------------
  case SysMsgCode of
    EnLockerNotExists: //������������
      begin
        FactroyPanel.Visible := False;
        Application.MessageBox(InsertUsbDog, Titel, MB_OK + MB_ICONSTOP + MB_TOPMOST);
      end;
    USBArrival, USBRemove: //USB�豸����
      begin

      end;
    GetComPortListOK: //��ȡ�б�ɹ�
      begin
        LoadSysComList(SysEventData); //�г�COM��
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
          Caption := CapStr[Tag]; //��ʾ���ѻ�
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
          Caption := CapStr[Tag]; //��ʾ������
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
    GraphicMaxSizeError: //�ӹ���Χ����
      begin
        ShowMessage(OutSize);
      end;
    RequestAndContinue: //�忨�����������Ƿ����
      if not IsRequestAndContinueDLGshow then
      begin
        IsRequestAndContinueDLGshow := True;

        //���ü����ӹ��Ľӿ�
        LoadBreakPiontData(Application.MessageBox(AskContinue, Titel, MB_YESNO + MB_ICONQUESTION + MB_DEFBUTTON2 + MB_TOPMOST) = ID_YES); //����ϵ���������
      end;
    ContinueWorking: //�����ӹ�
      begin
        PauseButton.Caption := cCaption[0];
        PauseButton.Tag := 0;
      end;
    MainCardRegisterOK: //�忨ע��ɹ�
      begin
        Label80.Caption := RegisterOK;
        CardIDtoServerBTN.Enabled := True;
      end;
    MainCardRegisterError: //�忨ע��ʧ��
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
        vEventData := vTms.Split([';']); //��;�ָ� ��ʽΪ�� �ļ����;�ļ����·��������
        WriteRZ(Format(DownLoadFileIndexTip, [vEventData[0], vEventData[1]]), Now);
      except
      end;
    MainCardIsPirate: //δ�����Ҫ����
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
                    const vUserName: PChar; //����
                    const vAddress: PChar; //��ַ
                    const vTelphone: PChar; //�绰
                    const vQQ: PChar; //QQ
                    const vWX: PChar; //WX
                    const vEmail: PChar; //����
                    const Country: PChar; //����
                    const Distributor: PChar; //������
                    const Trademark: PChar; //Ʒ��
                    const Model: PChar; //�ͺ�
                    const MachineID: PChar//����ID
                  }
                  vUserName := PChar(UserNameEdit.Text); //����
                  vAddress := PChar(AddressEdit.Text); //��ַ
                  vTelphone := PChar(TelphoneEdit.Text); //�绰
                  vQQ := PChar(QQEdit.Text); //QQ
                  vWX := PChar(WXEdit.Text); //WX
                  vEmail := PChar(EmailEdit.Text); //����
                  Country := PChar(CountryEdit.Text); //����
                  Distributor := PChar(DistributorEdit.Text); //������
                  Trademark := PChar(TrademarkEdit.Text); //Ʒ��
                  Model := PChar(ModelEdit.Text); //�ͺ�
                  MachineID := PChar(MachineIDEdit.Text); //����ID
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
            vUserName, //����
            vAddress, //��ַ
            vTelphone, //�绰
            vQQ, //QQ
            vWX, //WX
            vEmail, //����
            Country, //����
            Distributor, //������
            Trademark, //Ʒ��
            Model, //�ͺ�
            MachineID //����ID
          )
        else if vTms = 'mrRetry' then
        begin
          GetMainCardInfo;
        end;
      end;
    MainCardMachineMoreInfo: //���ذ忨������ע�ἰ������Ϣ
      begin
        //2020/9/1 13:07:42;2020/9/12 23:26:38;2020/9/12 21:03:26;2020/9/12 23:26:38;123063
        vTms := Trim(SysEventData);

        vEventData := vTms.Split([';']); //��;�ָ�

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
    ReturnWorkState: //���ص�ǰ����״̬
      begin
        //����״̬��ִ�е�ָ�X���꣬Y���꣬Z���꣬���⹦�ʣ��Ҷ�ֵ
        //iWorkState, iCmdCode, iXPos, iYPos, iZPos, iLaserPower, iGray: Integer;
        vTms := Trim(SysEventData);

        try
          vEventData := vTms.Split([';']); //��;�ָ�

          iWorkState := StrToIntDef(vEventData[0], 0); //����״̬
          iCmdCode := StrToIntDef(vEventData[1], 0); //ִ�е�ָ��
          iXPos := StrToFloatDef(vEventData[2], 0.0); //X����
          iYPos := StrToFloatDef(vEventData[3], 0.0); //Y����
          iZPos := StrToFloatDef(vEventData[4], 0.0); //Z����
          iLaserPower := StrToIntDef(vEventData[5], 0); //���⹦��
          iGray := StrToIntDef(vEventData[6], 0); //�Ҷ�ֵ

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
          CardIDEdit.Text := GetMainCardID; //���ذ忨ID
        end;
      end;
    FactroyPasswordValid: //����������ȷ
      begin
        FactroyPanel.Visible := True;
      end;
    FactroyPasswordInValid: //�����������
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
    FoundSoftNewVersion, FoundFirmwareNewVersion: //�����°汾
      begin
        IsFirmwareUpdate := SysMsgCode = FoundFirmwareNewVersion; //�ǹ̼���
        //
        //ShowMessage(JsonFile);
        ShowUpdateAskWindow(//�����°汾������ѯ���Ƿ�����
          IsFirmwareUpdate, //
          PChar(cFlag[UpdaeRadioGroup.ItemIndex]), //��ʶ��
          JsonFile, //������־�ļ�
          0, //����ǰ���еĳ������з�ʽ 0���в��ȴ����� 1���к�����ȴ�����ִ������(Ĭ��)
          BeforeRunExeName, //����ǰҪ���еĳ���֧��.BAT .EXE
          AfterRunExeName //������Ҫ������EXE
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
    UpdateFirmwareEnd: //�̼����³ɹ�
      begin
        if Assigned(UpdateFirmwareForm) then
          FreeAndNil(UpdateFirmwareForm);

        WriteRZ(UpdateFirmwareEndTip, Now);
      end;
    UpdateFirmwareTimeOut: //������ʱ
      begin
        if Assigned(UpdateFirmwareForm) then
          FreeAndNil(UpdateFirmwareForm);
      end;
    WorkFinished: //�ӹ����
      begin
        Inc(vAutoWorkCount);
        WriteAutoWorkInfo(Format('�� %d �μӹ���ɣ�׼���ٴ��ظ��ӹ�...', [vAutoWorkCount]));
        if AutoCheckBox.Checked then
          LoadImageFileBtn.Click;
      end;
    TimeConsuming: //�ӹ������ܼƺ�ʱʱ��
      begin
        WriteRZ(Format(TimeConsumingTip, [API_FormatMillisecondString(StrToIntDef(SysEventData, 0))]), Now);
      end;
    ReadParamsFromServerOK, ReadSysParamFromCardOK, WriteSysParamToCardOK: //��ȡ���ļĴ������� ���ظ�ʽΪ����ַ1,����1;��ַ2,����2;...��ַn,����n;
      begin
        vTms := Trim(SysEventData);

        try
          vEventData := vTms.Split([';']); //��;�ָ�

          for i := Low(vEventData) to High(vEventData) - 1 do
          begin
            vAddr := StrToIntDef(Format('%s', [LeftStr(vEventData[i], Pos(',', vEventData[i]) - 1)]), 300); //��ַ

            if vAddr in [1..128] then //
            begin
              vTms := Format('%s', [RightStr(vEventData[i], Length(vEventData[i]) - Pos(',', vEventData[i]))]);
              vData := StrToFloatDef(vTms, 0); //����
              //
              case vAddr of  //Ӧ�õ�����
                2: //�̼��汾
                  FirmwareVerLabel.Caption := GetMainHardVersion; //�̼��汾
                4: //���ʱY�������ٶ�
                  YCarveMinSpeedEdit.Text := vTms;
                5:
                  MoveWorkSpeed := Round(vData); //�����ƶ��ٶ�
                7:
                  RetOriginalSpeed := Round(vData); //��ԭ���ٶ�
                8:
                  try
                    WorkArea := Round(vData); //��������
                    StorageEdit8.ItemIndex := WorkArea - 1;
                    Label16.Caption := Format(WorkAreaTip, [WorkArea]);
                  except
                  end;
                9:
                  begin
                    XStepNum := (vData); //X �����峤��
                  end;
                10:
                  begin
                    YStepNum := (vData); //Y �����峤��
                  end;
                13:
                  begin
                    LStep := Round(vData); //�м��:ˮƽ���ؼ��
                  end;
                14:
                  begin
                    HStep := Round(vData); //�м�ࣺ��ֱ���ؼ��
                  end;
                31: //�� 1 ��ӹ�ԭ�� X
                  begin
                    XMoveToPos1 := (vData); //
                    //
                    XMoveToPostEdit1.Text := vTms; //FloatToStr(vData);
                  end;
                32:  //�� 1 ��ӹ�ԭ�� Y
                  begin
                    YMoveToPos1 := (vData); //
                    //
                    YMoveToPostEdit1.Text := vTms; //FloatToStr(vData);
                  end;
                33:  //�� 2 ��ӹ�ԭ�� X
                  begin
                    XMoveToPos2 := (vData); //
                    //
                    XMoveToPostEdit2.Text := vTms; //FloatToStr(vData);
                  end;
                34:  //�� 2 ��ӹ�ԭ�� Y
                  begin
                    YMoveToPos2 := (vData); //
                    //
                    YMoveToPostEdit2.Text := vTms; //FloatToStr(vData);
                  end;
                35:   //�� 3 ��ӹ�ԭ�� X
                  begin
                    XMoveToPos3 := (vData); //
                    //
                    XMoveToPostEdit3.Text := vTms; //FloatToStr(vData);
                  end;
                36:  //�� 3 ��ӹ�ԭ�� Y
                  begin
                    YMoveToPos3 := (vData); //
                    //
                    YMoveToPostEdit3.Text := vTms; //FloatToStr(vData);
                  end;
                38:
                  begin    //�ӹ�������
                    WorkSpaceWidth := Round(vData) div 256 div 256; //��λ
                    WorkSpaceHeight := Round(vData) and $FFFF; //��λ
                    //
                    MaxWorkSpaceSizeXEdit.Text := IntToStr(WorkSpaceWidth);
                    MaxWorkSpaceSizeYEdit.Text := IntToStr(WorkSpaceHeight);
                  end;
                39:
                  begin
                    PrinterDrawUnitEdit.Text := IntToStr(Round(vData)); //��ͼ��λ
                  end;
                40:
                  MoveMinSpeed := Round(vData); //�����ƶ������ٶ�
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

//ϵͳ�ص����==================================================================================================

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
  CurrentVersion := API_GetVertionNumber(CompileDT); //�ѱ�������תΪ�汾��
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
  //ע��ص�����
  ProgressCallBack(vProgress);
  SysMessageCallBack(vSysMessage);
  //
  try
    siLang.LoadAllFromFile(ExtractFilePath(ParamStr(0)) + 'Languge.sil', True);
  {$IFDEF ENGLISHVER}
    SetLanguge(0); //Ӣ��
    SetFactroyType('DemoEng'); //����Ӣ�İ�DEMOר������(ƥ������ͨ����)

    siLangDispatcher.Language := siLangDispatcher.LangNames[1];
    StoragePanel.Free;
  {$ELSE}
    SetLanguge(1); //����
    SetFactroyType('Demo'); //�������İ�DEMOר������(ƥ������ͨ����)

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
  LogMemo.Lines.Append(GetAPILibCompileInfo); //��ȡAPI�ı���ģʽ
  PageControl.ActivePageIndex := 1;
  //
  cApplicationPath := ExtractFilePath(ParamStr(0));
  BeforeRunExeName := PChar(Format('%s\BeforeRun.bat', [ExtractFilePath(ParamStr(0))])); //����ǰ����(��ѡ)
  AfterRunExeName := PChar(ParamStr(0)); //����������(һ��Ϊ��EXE)
  JsonFile := PChar(Format('%sVersionLog.Json', [TPath.GetTempPath])); //������TEMP�ļ�����
end;

procedure TMainForm.FormShow(Sender: TObject);
begin
  LoadSysComList; //�г�COM��
end;

procedure TMainForm.GetMainHWNDClick(Sender: TObject);
var
  vCDRHwnd: Integer; //CDR�����ھ��
begin
  {ʾ������ΰ�CDR��������Ϊ����״̬}
  //�����ṩ�ĺ�������ȡCDR������ʵ���ľ��
  vCDRHwnd := GetCDRMainFormHWND(PChar('D:\Program Files\Corel\CorelDRAW Graphics Suite X8\Programs64\CorelDRW.exe'));

  if vCDRHwnd > 0 then //CDR�����ھ�����ڻ�ȡ�ɹ�
  begin
    //��һ������ʾ����
    ShowWindow(//ϵͳAPI���ú�������ָ�����ڵ���ʾ״̬
      vCDRHwnd, //CDR�����ھ��
      SW_NORMAL//���ڵ���ʾ��ʽΪ������״̬(�����Ƿ���ʾ����������������һ�ɻָ���������ʾ)
    );

    //�ڶ�����������ǰ
    BringWindowToTop(//ϵͳAPI���ú�����ָ���Ĵ������õ�Z��Ķ������������Ϊ���㴰�ڣ���ô��ڱ�����������Ϊ�Ӵ��ڣ�����Ӧ�Ķ��������ڱ����
      vCDRHwnd //CDR�����ھ��
    ); //
  end
  else //��ȡʧ�ܺ󣬷���ֵ
    case vCDRHwnd of
      0:
        ShowMessage('��ȡ CDR �����ھ��ʧ�ܣ�');
      -1:
        ShowMessage('ָ��EXE�ļ������ڣ�');
      -2:
        ShowMessage('ָ��EXE�ļ�û�����У�');
    end;
end;

procedure TMainForm.ImageOpenDialogSelectionChange(Sender: TObject);
begin
  with TOpenDialog(Sender) do
  begin
    InitialDir := ExtractFilePath(FileName)
  end;
end;

//�����������==================================================================================================
procedure TMainForm.StartWorkBtnClick(Sender: TObject); //��ʼ�ӹ�
begin
  if vPageCount > 0 then
  begin
    PauseButton.Caption := cCaption[0]; //'��ͣ';
    PauseButton.Tag := 0;

    StartMachining(not ZeroPointStyleCheckBox.Checked); //�ӹ�ԭ�㶨�壺True:��еԭ��Ϊԭ�� False:��ǰ�����Ϊԭ��;
  end;
end;

procedure TMainForm.Panel8Click(Sender: TObject);
begin
  vAutoWorkCount := 0;
end;

procedure TMainForm.PauseButtonClick(Sender: TObject); //��ͣ����
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

procedure TMainForm.StopButtonClick(Sender: TObject);  //ֹͣ�ӹ�
begin
  StopMachining;
  GrayImage.Picture.Graphic := nil;
  ProgressBar.Position := 0;
end;

procedure TMainForm.Button9Click(Sender: TObject); //��ԭ��   RetOriginalSpeed
begin
  LPenMoveToOriginalPoint(RetOriginalSpeed);
end;

procedure TMainForm.ActivationCodeEditDblClick(Sender: TObject);
begin
  ActivationCodeEdit.PasteFromClipboard; //˫������ճ��������
end;

procedure TMainForm.Button10MouseDown(Sender: TObject; Button: TMouseButton; Shift: TShiftState; X, Y: Integer); //����
begin
  TestLaserLight(True);  //������
end;

procedure TMainForm.Button10MouseUp(Sender: TObject; Button: TMouseButton; Shift: TShiftState; X, Y: Integer);  //����
begin
  TestLaserLight(False);  //�ؼ���
end;

procedure TMainForm.Button34Click(Sender: TObject); //���ص��
begin
  ControlMotor(True); //���ص��
end;

procedure TMainForm.Button2Click(Sender: TObject);  //ж�ص��
begin
  ControlMotor(False); //ж�ص��
end;

procedure TMainForm.Button22Click(Sender: TObject);
begin
  GetDeviceWorkState; //���ص�ǰ����״̬���ص��ж�ȡ��
end;

procedure TMainForm.Button23Click(Sender: TObject);
begin
  ShowAboutWindow; //������Ϣ
end;

//�����������==================================================================================================

function CreateRandomNumber(const Len: Word): string; //����ָ�����ȵ���������ַ���
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

    if ((Length(Result) < 3) and (Ord(TmsChar) < 65)) or ((Length(Result) > 2) and (Ord(TmsChar) > 58)) then //��λ����Ϊ0
      Continue;

    Result := Trim(Result + TmsChar);
  until Length(Result) = Len;
end;

procedure TMainForm.Button29Click(Sender: TObject);
begin
  FactoryPassWordEdit.Text := CreateRandomNumber(10);  //6λ���������Ϊ����
end;

procedure TMainForm.Button30Click(Sender: TObject);
begin
  WriteFactoryPassWord(//
    PChar(Trim(OLDFactoryPassWordEdit.Text)), //
    PChar(Trim(FactoryPassWordEdit.Text))); //���ó�������
end;

procedure TMainForm.Button31Click(Sender: TObject);
begin
  vLanguage := TButton(Sender).Tag;
  siLangDispatcher.Language := siLangDispatcher.LangNames[vLanguage];
end;

procedure TMainForm.CHECKFCPSWBTNClick(Sender: TObject);
begin
  CheckFactoryPassWord(PChar(Trim(TESTFactoryPassWordEdit.Text))); //StrToInt64Def(Trim(FactoryPassWordEdit.Text), 0)); //��֤��������
end;

procedure TMainForm.RSTFPSWBTNClick(Sender: TObject);
begin
  ResetFactoryPassWord(PChar('{B1946B01-30FA-4910-9DF0-495D74071D51}')); //��ʼ����������
end;

procedure TMainForm.Button33Click(Sender: TObject); //��ռĴ��������
begin
  StorageDataEdit.Clear
end;

//�и�������==================================================================================================
procedure TMainForm.LoadSvgFile(const FileName: TFileName);//����SVG
begin

end;

procedure TMainForm.LoadPltFile(const FileName: TFileName);//����PLT
var
  vGobjectCount, vInvalidGobjectCount: Integer;
  MinSpeed, Runspeed, MoveSpeed: Integer; //��ʼ�ٶȣ������ٶȣ������ٶȣ�X �����峤�ȱȣ�Y �����峤�ȱ�
  MinSpeedPower, RunSpeedPower, CarvePower: Word; //�����������ٱȡ��и��
begin
  if FileExists(FileName) then
  begin
    //��ֵ
    //======================================================================
    MinSpeed := StrToIntDef(Trim(CutMinSpeedEdit.Text), 20); //��ʼ�ٶ�
    Runspeed := StrToIntDef(Trim(CutRunspeedEdit.Text), 40); //�����ٶ�
    MoveSpeed := StrToIntDef(Trim(StorageEdit5.Text), 200); //�����ٶ�
    //
    MinSpeedPower := StrToIntDef(Trim(CutMinSpeedPowerEdit.Text), 10); //�����ٱ�
    RunSpeedPower := StrToIntDef(Trim(CutRunSpeedPowerEdit.Text), 20); //�����ٱ�
    CarvePower := StrToIntDef(Trim(CutPowerEdit.Text), 40); //�и��
    //======================================================================
    //
    {
      const vFileName: PChar; //�����ļ���
      const FinishRun: word; //�ӹ���ɺ�ִ�е�ָ��(ʹ��32Bit�����Ʊ�ʾ�趨״̬)
      const ZeroPointStyle: Boolean; //�ӹ�ԭ�㶨�壺True:��еԭ��Ϊԭ�� False:��ǰ�����Ϊԭ��
      const XYZStyle: Byte; //XYZ�᷽ʽ��0:XY 1:XZ
      //
      const vMinSpeed, vRunspeed, vMoveSpeed: Double; //��ʼ�ٶȣ������ٶȣ������ٶ�
      const MinSpeedPower, RunSpeedPower, CarvePower: word; //�����������ٱȡ��и��
      //
      var GobjectCount: Integer; //��Чͼ�ζ�������
      var InvalidGobjectCount: Integer //��Чͼ�ζ�������
    }
    vPageCount := CreateMicroStepsDataEx(//
      PChar(FileName), //PLT�ļ���
      BinToDec(IntToStr(FinishRunComtBox.ItemIndex)), //�ӹ���ɺ�ִ�е�ָ��(ʹ��32Bit�����Ʊ�ʾ�趨״̬) ����ѡ����λ1�ֽڣ�10���Ʊ�ʾ����256�ֶ�����
      not ZeroPointStyleCheckBox.Checked, //�ӹ�ԭ�㶨�壺True:��еԭ��Ϊԭ�� False:��ǰ�����Ϊԭ��
      0, //XYZ�᷽ʽ��0:XY 1:XZ
      //
      //XStepNum, YStepNum, //X �����峤�ȣ�Y �����峤��
      //
      MinSpeed, Runspeed, MoveSpeed,  //��ʼ�ٶȣ������ٶȣ������ٶ�
      MinSpeedPower, RunSpeedPower, CarvePower, //�����������ٱȡ��и��
      //
      vGobjectCount,   //��Чͼ�ζ�������
      vInvalidGobjectCount//��Чͼ�ζ�������
    ); //��ȡָ���ļ����Ķ��߶�ͼ�����ݣ��������ݰ�����
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
      StartWorkBtn.Enabled := True; //ʹ�ܿ�ʼ
      //
      WorkState := 1;
      //
      ProgressLabel.Caption := Format(//
        '���� %d ��ͼ�Σ����곬��ͼ�� %d ������ʵ�ʿɼӹ�ͼ�� %d ��', //
        [vGobjectCount, vInvalidGobjectCount, vGobjectCount - vInvalidGobjectCount] //
      );
    end
    else
      ProgressLabel.Caption := '��ͼ���ݴ���...';
  end;
end;
//�и�������==================================================================================================


//��̲������==================================================================================================

procedure TMainForm.LoadPicFile(const FileName: TFileName);//����ͼ��
var
  //======================================================================
  vGrayImage: TFileName;   //�Ҷ�ͼ���ļ���
  //======================================================================
  vStartX, vStartY: Double; //�����ʼλ�ã�mm

  //======================================================================
  vImageDPI: Double; //ͼ��ֱ���
  vIWidth, vIHeight: Double; //ͼ���ߣ���λmm
  vUnits: Integer; //�ֱ��ʵ�λ 0=����/Ӣ�� 1=����/����
  //======================================================================
  vCarvePower: Word; //��̹���
  vMinSpeedPower, vRunSpeedPower: Word; //�����������ٱ�
  vCarvePixelsX, vCarvePixelsY: Double; //���ͼ�������� �ɷ��� (���е���)
const
  vMod: array[Boolean] of Byte = (0, 1);
  CarveStyle: array[Boolean] of string = ('01', '00'); //00�����е��  01�����е��
begin
  //��ֵ
  //======================================================================
  vIWidth := StrToFloatDef(Trim(IWidthEdit.Text), 50); //���ͼ����:��λmm
  vIHeight := StrToFloatDef(Trim(IHeightEdit.Text), 50); //���ͼ��߶�:��λmm
  //
  vStartX := StrToFloatDef(Trim(IStartXEdit.Text), 0); //��ʼλ��X mm
  vStartY := StrToFloatDef(Trim(IStartYEdit.Text), 0); //��ʼλ��Y mm
  //
  vImageDPI := StrToFloatDef(Trim(GraphicDPIComBox.Text), 300); //ͼ��ֱ���
  vUnits := UnitsComboBox.ItemIndex; //�ֱ��ʵ�λ0=����/Ӣ�� 1=����/����
  //
  vMinSpeedPower := StrToIntDef(Trim(CarveMinSpeedPowerEdit.Text), 1);
  vRunSpeedPower := StrToIntDef(Trim(CarveRunSpeedPowerEdit.Text), 5); //�����������ٱ�
  //
  vCarvePower := StrToIntDef(Trim(CarvePowerEdit.Text), 100); //��̹���
  //
  if FileExists(FileName) then
  begin
    vGrayImage := Format('%sTmpFile\%s', [cApplicationPath, ExtractFileName(ChangeFileExt(FileName, '.jpg'))]);
    //
    vPageCount := CreateCarvePictureDataEx(//
      False, //�Ƿ�������ݣ�TrueΪ��գ������������������Ҫ����ΪFalse��ʹ֮��Ϊ׷��״̬
      False, //�ӹ���ɺ��߱߿�
      PChar(FileName), PChar(vGrayImage), //Դ�ļ���Ŀ���ļ�
      False, //���������ݺ��Ƿ����ϼӹ�
      BinToDec(IntToStr(FinishRunComtBox.ItemIndex)), //�ӹ���ɺ�ִ�е�ָ��(ʹ��32Bit�����Ʊ�ʾ�趨״̬) ����ѡ����λ1�ֽڣ�10���Ʊ�ʾ����256�ֶ�����
      False, //�ӹ�ԭ�㶨�壺True:��еԭ��Ϊԭ�� False:��ǰ�����Ϊԭ��
      0, //XYZ�᷽ʽ��0:XY 1:XZ
      //XStepNum, YStepNum, //���峤��
      True,  //�Ƿ������̣�TRUEΪ������
      0, //0=���е�̣�1=���е��
      HStep, LStep, //���м�� 1-100
      StrToIntDef(Trim(ErrXEdit.Text), 50), //�����϶�� 0-500
      vMinSpeedPower, vRunSpeedPower, //�����������ٱ�
      StrToFloatDef(Trim(XCarveMinSpeedEdit.Text), 30), //�����ʼ�ٶ� mm/s Ĭ��30
      StrToFloatDef(Trim(CarveRunSpeedEdit.Text), 180), //����ٶ� mm/s Ĭ��180
      vCarvePower, //��̹���
      DefaultDPICheckBox.Checked, //�Ƿ�ԭͼ�ߴ����
      0, //ͼ����ת��ʽ
      vMod[BWCheckBox.Checked], //ͼ����ʽ 0 �Ҷ�ͼ 1 ��ֵͼ
      vStartX, vStartY, //������ mm
      //
      vCarvePixelsX, vCarvePixelsY, //���ͼ��������  �ɷ���
      vImageDPI, //ͼ��ֱ��� 50-300 �ɷ���
      vUnits, //�ֱ��ʵ�λ0=����/Ӣ�� 1=����/����  �ɷ���
      vIWidth, vIHeight//��ӡͼ���ߣ�mm���ɷ���
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
    GraphicDPIComBox.Text := Format('%g', [vImageDPI]); //ͼ��ֱ���
    UnitsComboBox.ItemIndex := vUnits; //�ֱ��ʵ�λ0=����/Ӣ�� 1=����/����
    IWidthEdit.Text := Format('%g', [vIWidth]); //��ӡͼ���mm��
    IHeightEdit.Text := Format('%g', [vIHeight]); //��ӡͼ��ߣ�mm��
  end;

  //�����Ѿ���ɻҶ����ݵĴ������׼���·���
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
      StartWorkBtn.Enabled := True; //ʹ�ܿ�ʼ���
      //
      {$IFDEF WRITELOG}
      WriteRZ(//
        Format(//
        '��ǰ��̵�ͼ���� %g �� %g �У������Ϊ %d ������', //
        [vCarvePixelsY, vCarvePixelsX, vPageCount] //
      ), Now);
      WriteRZ(vLine, Now, False);
      {$ENDIF}
    end
    else
    begin
      StartWorkBtn.Enabled := False;
      ProgressLabel.Caption := '�������̫С��';
    end;
  end
  else
  begin
    GrayImage.Picture := nil;
  end;
end;
//��̲������==================================================================================================

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
    StartWorkBtn.Enabled := True; //ʹ�ܿ�ʼ
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
    StartWorkBtn.Enabled := True; //ʹ�ܿ�ʼ
    //
    WorkState := 1;
  end;
end;

procedure TMainForm.LoadImageFileBtnClick(Sender: TObject);
var
  vFileName: TFileName;
begin
  TThread.CreateAnonymousThread( //ʹ�ö��߳�
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
        0: //����
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
        1: //���
          with PLTOpenDialog do
            if Execute then
            begin
              DataCheckListBox.Items.Append(FileName);
            end;
        2: //ɾ��
          begin
            DataCheckListBox.DeleteSelected;
          end;
        3: //���
          begin
            DataCheckListBox.Clear;
          end;
        4: //ȫѡ
          begin
            for i := 0 to DataCheckListBox.Items.Count - 1 do
              DataCheckListBox.Checked[i] := True;
          end;
        5: //��ѡ
          begin
            for i := 0 to DataCheckListBox.Items.Count - 1 do
              DataCheckListBox.Checked[i] := not DataCheckListBox.Checked[i];
          end;
      end;
      TButton(Sender).Enabled := True;
    end).Start;
end;

//�Ĵ����������==================================================================================================
procedure TMainForm.Button1Click(Sender: TObject);
begin
  StorageAddEdit.Clear
end;

procedure TMainForm.Button43Click(Sender: TObject);
begin
  WriteSysParamToCard( //��㱣�浽�Ĵ���
    PChar('4,13,14'), //��ַ (ֱ�������ַ����,�ָ�)
    PChar(Format(//ֵ (ֱ�������ַ��Ӧ˳���ֵ����,�ָ�)
    '%d,%d,%d', [//
    StrToInt64Def(Trim(YCarveMinSpeedEdit.Text), 10000), //���ʱY�������ٶ� 1000-200000�ķ�Χֵ ����1000����mm��λ
    StrToInt64Def(Trim(StorageEdit13.Text), 5), //
    StrToInt64Def(Trim(StorageEdit14.Text), 100)]//
  ))//
  );
end;

procedure TMainForm.Button44Click(Sender: TObject);
begin
  ReadSysParamFromCard('4,13,14'); //����ȡ�Ĵ���(ֱ�������ַ����,�ָ�)
end;

procedure TMainForm.Button45Click(Sender: TObject);
begin
  if RegisterMainCard(PChar(Trim(ActivationCodeEdit.Text))) <> '' then //ע��
    ShowMessage(RegisterMainCardTip);
end;

procedure TMainForm.Button46Click(Sender: TObject);
begin
  GetMainCardRegState
end;

procedure TMainForm.ShowUpdateAskWindow(//�����°汾������ѯ���Ƿ�����
  const IsFirmwareUpdate: Boolean; //�Ƿ�̼����£���Ϊ�������
  const Flag: string; //��ʶ��
  const vJsonFile: string; //
  const BeforeRunType: Integer; //����ǰ���еĳ������з�ʽ 0���в��ȴ����� 1���к�����ȴ�����ִ������(Ĭ��)
  const BeforeRunExeName: string; //������ʼǰ���еĳ����ļ�����Ϊ���������겻����
  const AfterRunExeName: string //������ɺ����еĳ����ļ�����Ϊ���������겻����
);
var
  UpdateTips: string; //��־����
  RetVersion: Integer; //JSON�з��ص��°汾��
  AJson: TQJson;
begin
  UpdateTips := '';
  {=======================================
  �ڴ˽���JSON�ļ���������������ʾ������־
  =======================================}
  if FileExists(vJsonFile) then //���°汾����������־�ļ���Ҫ���һ��
  try
    //����PLT
    AJson := TQJson.Create;
    try
      AJson.LoadFromFile(vJsonFile);
      UpdateTips := AJson.ForcePath('UpdateLog').AsString; //������־
      RetVersion := AJson.ForcePath('ProductVersion').AsInteger; //�°汾��
    finally
      FreeAndNil(AJson);
      DeleteFile(vJsonFile); //������JSON��LOG�ļ�����ɾ����
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
    StartVersionUpdate(//��������汾
      IsFirmwareUpdate, //�Ƿ�̼����£���Ϊ�������
      PChar(cFlag[UpdaeRadioGroup.ItemIndex]), //��ʶ��
      '', //�����ļ�
      0, //����Ϊ���°�
      BeforeRunType, //����ǰ���еĳ������з�ʽ 0���в��ȴ����� 1���к�����ȴ�����ִ������
      PChar(BeforeRunExeName), //����ǰҪ���еĳ���֧��.BAT .EXE
      PChar(AfterRunExeName) //������Ҫ������EXE
    );
  end;
end;

procedure TMainForm.Button64Click(Sender: TObject);
const
  cFilter: array[Boolean] of string = (//
    '���������(*.tar)|*.tar', //
    '�̼�������(*.bin;*.cab)|*.bin;*.cab' //
    );
begin
  {
    function StartVersionUpdate(//�������
      const IsFirmwareUpdate: Boolean; //�Ƿ�̼����£���Ϊ�������
      const Flag: PChar; //��ʶ��
      const LocalFile: PChar; //���������ļ�����������ʱ��Ч��
      const UpdateVersion: Integer; //Ҫ�����İ汾�� ָ����ʽΪ201201�������˰汾��ֵΪ0�����������°�
      const BeforeRunType: Integer; //����ǰ���еĳ������з�ʽ 0���в��ȴ����� 1���к�����ȴ�����ִ������(Ĭ��)
      const BeforeRunExeName: PChar; //������ʼǰ���еĳ����ļ�����Ϊ���������겻����
      const AfterRunExeName: PChar //������ɺ����еĳ����ļ�����Ϊ���������겻����
    ): Integer; stdcall; external DllFile;
  }
  IsFirmwareUpdate := TButton(Sender).Tag = 1; //1Ϊ�̼�

  case UpdaeRadioGroup.ItemIndex of
    0: //���ع̼�
      with BinOpenDialog do
      begin
        Filter := cFilter[IsFirmwareUpdate];
        if Execute() then
        begin
          StartVersionUpdate(//��������汾
            IsFirmwareUpdate, //�Ƿ�̼����£���Ϊ�������
            PChar(cFlag[UpdaeRadioGroup.ItemIndex]), //��ʶ��
            PChar(FileName), //���������ļ�
            0, //Ҫ�����İ汾�� ָ����ʽΪ201201�������˰汾��ֵΪ0�����������°�
            0, //����ǰ���еĳ������з�ʽ 0���в��ȴ����� 1���к�����ȴ�����ִ������(Ĭ��)
            PChar(BeforeRunExeName), //����ǰҪ���еĳ���֧��.BAT .EXE
            PChar(AfterRunExeName) //������Ҫ������EXE
          );
        end;
      end;
    1, 2: //����̼�
      begin
        {
          procedure CheckVersionUpdate(//��ѯ�Ƿ����°汾
            const IsFirmwareUpdate: Boolean; //�Ƿ�̼����£���Ϊ�������
            const Flag: PChar; //��ʶ��
            const CurrentVersion: Integer; //��ǰ�汾��
            const VersionNoteToJsonFile: string //Json��ʽ�ĸ��������ļ�����ֵΪ�ձ�ʾ������Ҳ�������ļ�
          ); stdcall; external DllFile;//ͨ���ص����������°汾���
        }
        case IsFirmwareUpdate of
          False: //�������
            CurrentVersion := API_GetVertionNumber(CompileDT); //����汾
          True: //�̼�����
            CurrentVersion := StrToIntDef(GetMainHardVersion, 201201); //�̼��汾
        end;
        //ShowMessage('JsonFile = ' + JsonFile);
        CheckVersionUpdate(//
          IsFirmwareUpdate, //
          PChar(cFlag[UpdaeRadioGroup.ItemIndex]), //��ʶ��
          CurrentVersion, //��ǰ�汾��
          PChar(JsonFile)//Json��ʽ�ĸ��������ļ�����ֵΪ�ձ�ʾ������Ҳ�������ļ�
        );
      end
  end;
end;

procedure TMainForm.Button66Click(Sender: TObject);
begin
  SetLanguge(TButton(Sender).Tag); //ʹ�������л�
end;

procedure TMainForm.Button68Click(Sender: TObject);
var
  MinSpeed, Runspeed, MoveSpeed: Integer; //��ʼ�ٶȣ������ٶȣ������ٶ�
  MinSpeedPower, RunSpeedPower, CarvePower: Word; //�����������ٱȡ��и��
begin
  //��ֵ
  //======================================================================
  MinSpeed := StrToIntDef(Trim(CutMinSpeedEdit.Text), 20); //��ʼ�ٶ�
  Runspeed := StrToIntDef(Trim(CutRunspeedEdit.Text), 40); //�����ٶ�
  MoveSpeed := StrToIntDef(Trim(StorageEdit5.Text), 200); //�����ٶ�
  //
  MinSpeedPower := StrToIntDef(Trim(CutMinSpeedPowerEdit.Text), 10); //�����ٱ�
  RunSpeedPower := StrToIntDef(Trim(CutRunSpeedPowerEdit.Text), 20); //�����ٱ�
  CarvePower := StrToIntDef(Trim(CutPowerEdit.Text), 40); //�и��
  //======================================================================
  {
    procedure LaserDrawRectangle(//
      const XYZStyle: Byte; //XYZ�᷽ʽ��0:XY 1:XZ
      const ZeroPointStyle: Boolean; //�ӹ�ԭ�㶨�壺True:��еԭ��Ϊԭ�� False:��ǰ�����Ϊԭ��
      const LaserOn: Boolean; //�Ƿ񿪼���
      const Left, Top, Right, Bottom: Integer; //����������
      const vMinSpeed, vRunspeed, vMoveSpeed: Double; //��ʼ�ٶȣ������ٶȣ������ٶ�
      const CarvePower, MinSpeedPower, RunSpeedPower: Word //�и�������������ٱ�
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
  GetMainCardInfo; //��ѯ��ǰ�����İ忨���󶨻���������ϸ����
end;

procedure TMainForm.Button4Click(Sender: TObject);
begin
  ReadSysParamFromCard(PChar(StorageAddEdit.Text)); //��ȡ�Ĵ���
end;

procedure TMainForm.Button5Click(Sender: TObject);
begin
  WriteSysParamToCard(//��㱣�浽�Ĵ���
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
    PrinterDrawUnitEdit.Text := '1016'; //��ͼ��λ
  finally
    SaveParmBtnClick(nil); //���浽�Ĵ���
  end;
end;

procedure TMainForm.Button36Click(Sender: TObject);
begin
  ReadSysParamFromCard(PChar('3,7,8,9,10,21,22,38,39')); //����ȡ�Ĵ���(ֱ�������ַ����,�ָ�)
end;

procedure TMainForm.Button37Click(Sender: TObject);
begin
  WriteSysParamToCard( //��㱣�浽�Ĵ���
    PChar('3,7,8,9,10,21,22,38,39'), //��ַ (ֱ�������ַ����,�ָ�)
    PChar(//
    Format(//ֵ (ֱ�������ַ��Ӧ˳���ֵ����,�ָ�)
    '%g,%g,%d,%g,%g,%g,%g,%d,%d', [//
    StrToFloatDef(Trim(StorageEdit3.Text), 5), //
    StrToFloatDef(Trim(StorageEdit7.Text), 100), //
    StorageEdit8.ItemIndex + 1, //��������
    StrToFloatDef(Trim(StorageEdit9.Text), 6.329114), //
    StrToFloatDef(Trim(StorageEdit10.Text), 6.329114), //
    StrToFloatDef(Trim(StorageEdit21.Text), 0), //
    StrToFloatDef(Trim(StorageEdit22.Text), 0), //
    StrToIntDef(Trim(MaxWorkSpaceSizeXEdit.Text), 320) * 256 * 256 + //��λ
    StrToIntDef(Trim(MaxWorkSpaceSizeYEdit.Text), 210), //��λ
    StrToIntDef(Trim(PrinterDrawUnitEdit.Text), 1016) //��ͼ��λ
    ]//
  )//
  ));
end;

procedure TMainForm.Button26Click(Sender: TObject);
begin
  WriteSysParamToCard( //��㱣�浽�Ĵ���
    PChar('31,32,33,34,35,36'), //��ַ (ֱ�������ַ����,�ָ�)   4294.967295
    PChar(//
    Format(//ֵ (ֱ�������ַ��Ӧ˳���ֵ����,�ָ�)
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
  ReadSysParamFromCard(PChar('31,32,33,34,35,36')); //����ȡ�Ĵ���(ֱ�������ַ����,�ָ�)
end;

procedure TMainForm.ReLoadParmBtnClick(Sender: TObject); //��ȡ����
begin
  ReadSysParamFromCard(PChar('5,11,12,23,24,25,27,40')); //����ȡ�Ĵ���(ֱ�������ַ����,�ָ�)
end;

procedure TMainForm.ResetFacPswBTNClick(Sender: TObject);
begin
  RSTFPSWBTN.Click;
  CHECKFCPSWBTN.Click;
end;

procedure TMainForm.Button19Click(Sender: TObject);
begin
  ReadSysParamFromCard('15,16,17'); //����ȡ�Ĵ���(ֱ�������ַ����,�ָ�)
end;

procedure TMainForm.Button60Click(Sender: TObject);
begin
  CardIDEdit.SelectAll;
  CardIDEdit.CopyToClipboard;
  ShowMessage(CopyToClipboardTip);
end;

procedure TMainForm.Button6Click(Sender: TObject);
begin
  ReadSysParamFromCard(PChar('18,19,20')); //����ȡ�Ĵ���(ֱ�������ַ����,�ָ�)
end;

procedure TMainForm.Button11Click(Sender: TObject);
begin
  WriteSysParamToCard( //��㱣�浽�Ĵ���
    PChar('18,19,20'), //��ַ (ֱ�������ַ����,�ָ�)
    PChar(Format(//ֵ (ֱ�������ַ��Ӧ˳���ֵ����,�ָ�)
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
  GetDeviceID(False); //��ȡ����ΨһID(�ص��з���)
end;

procedure TMainForm.Button51Click(Sender: TObject);
begin
  GetClientAddr(False); //��ȡ����������IP��ַ(�ص��з���)
end;

procedure TMainForm.Button52Click(Sender: TObject);
begin
  WriteSysParamToCard( //��㱣�浽�Ĵ���
    PChar('15,16,17'), //��ַ (ֱ�������ַ����,�ָ�)
    PChar(Format(//ֵ (ֱ�������ַ��Ӧ˳���ֵ����,�ָ�)
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
  SaveMainBoardParamsToServer  //����������������
end;

procedure TMainForm.Button54Click(Sender: TObject);
begin
  ReadMainBoardParamsFromServer; //�ӷ�������ȡ�û��忨���ò���
end;

procedure TMainForm.Button55Click(Sender: TObject);
begin
//��ȡ��������Ϣ
//���أ�������;��ַ;�������;�ۺ�绰;QQ����;΢�ź���;��������;��ַ;ApplicationID
  ShowMessage(GetHardwareKeyInfo); //
end;

procedure TMainForm.Button56Click(Sender: TObject);
begin
  LaserTubeZeroClearing(PChar(ResetPassWordEdit.Text)); //�����ʹ��ʱ������
end;

procedure TMainForm.Button57Click(Sender: TObject);
begin
  if ExportSFileFromCDR('D:\Demo.SVG') = 0 then
    ShowMessage('����SVG�ļ��ɹ���')
  else
    ShowMessage('����SVG�ļ�ʧ�ܣ�')
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
    ShowMessage('�и�ʧ�ܣ�')
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
    ShowMessage('���ʧ�ܣ�')
end;

procedure TMainForm.SaveParmBtnClick(Sender: TObject); //�������
begin
  WriteSysParamToCard( //��㱣�浽�Ĵ���
    PChar('5,11,12,23,24,25,27,40'), //��ַ (ֱ�������ַ����,�ָ�)
    PChar(//
    Format(//ֵ (ֱ�������ַ��Ӧ˳���ֵ����,�ָ�)
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
    SaveParmBtnClick(nil); //���浽�Ĵ���
  end;
end;
//�Ĵ����������==================================================================================================

//ϵͳ�������==================================================================================================

procedure TMainForm.ComButtonClick(Sender: TObject); //���� �ѻ�
begin
  case TButton(Sender).Tag of
    0:
      begin
        InitComPort(GetComPortName(ComPortBox.Text)); //1��ʾcom1������
      end
  else
    begin
      UnInitComPort;
    end;
  end;
end;

procedure TMainForm.Button7Click(Sender: TObject); //�������ӹ�����
begin
  SetSoftwareInitialization(//�������ӹ�����
    StrToIntDef(Trim(PrinterDrawUnitEdit.Text), 1016), //
    0, 0, //
    StrToIntDef(Trim(MaxWorkSpaceSizeXEdit.Text), 297), //
    StrToIntDef(Trim(MaxWorkSpaceSizeYEdit.Text), 210)//
  );
end;

procedure TMainForm.Button8Click(Sender: TObject);
begin
  CalibrateGuideExecute; //���У׼��
end;

//ϵͳ�������==================================================================================================



//�����ƶ����==================================================================================================

procedure TMainForm.StorageEdit5Change(Sender: TObject);  //�����ƶ��ٶ�
begin
  MoveWorkSpeed := StrToIntDef(Trim(StorageEdit5.Text), 100);
end;

procedure TMainForm.TrackBar1Change(Sender: TObject);
begin
  StorageEdit16.Text := IntToStr(TrackBar1.Position);
end;

//XYZ�᷽ʽ��0:XY 1:XZ ZeroPointStyle �ӹ�ԭ�㶨�壺True:��еԭ��Ϊԭ�� False:��ǰ�����Ϊԭ��
procedure RunMove(const XYZ: Boolean; const ZeroPointStyle: Boolean; mTag: Integer; mXStep, mYStep: Double);//�ƶ���ָ��λ�� XYZ = True�������ƶ���XZ  False�������ƶ���XY
begin
  try
    case mTag of
      1: //����
        begin
          MoveToX := MoveToX - mXStep;
          MoveToY := MoveToY + mYStep;
        end;
      2: //��
        begin
          MoveToY := MoveToY + mYStep;
        end;
      3: //����
        begin
          MoveToX := MoveToX + mXStep;
          MoveToY := MoveToY + mYStep;
        end;
      4: //��
        begin
          MoveToX := MoveToX - mXStep;
        end;
      5: //��ԭ��
        begin

        end;
      6: //��
        begin
          MoveToX := MoveToX + mXStep;
        end;
      7: //����
        begin
          MoveToX := MoveToX - mXStep;
          MoveToY := MoveToY - mYStep;
        end;
      8: //��
        begin
          MoveToY := MoveToY - mYStep;
        end;
      9: //����
        begin
          MoveToX := MoveToX + mXStep;
          MoveToY := MoveToY - mYStep;
        end;
    end;
    //
    {
      WorkArea: Byte = 1; //��������
      WorkSpaceWidth, WorkSpaceHeight: Word; //�ӹ�������
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
      //�ӹ�ԭ�㶨�壺True:��еԭ��Ϊԭ�� False:��ǰ�����Ϊԭ�� Ŀ�������Ϊ�з�������
      LPenQuickMoveTo(0, True, (MoveToX), (MoveToY), 0, MoveMinSpeed, MoveWorkSpeed); //�����ƶ���ָ�������
    end
  except
  end;
end;

procedure MoveZ(const mTag: Integer; mZStep: Double);
begin
  case mTag of
    2: //��
      begin
        MoveToZ := MoveToZ + mZStep;
      end;
    8: //��
      begin
        MoveToZ := MoveToZ - mZStep;
      end;
  end;
  if MoveToZ > 0 then
    MoveToZ := 0;

  MainForm.PosLabel.Caption := Format('X = %g Y = %g Z = %g', [MoveToX, MoveToY, MoveToZ]);
  //XYZ�᷽ʽ��0:XY 1:XZ �ӹ�ԭ�㶨�壺True:��еԭ��Ϊԭ�� False:��ǰ�����Ϊԭ�� Ŀ�������Ϊ�з�������
  LPenQuickMoveTo(1, True, (MoveToX), (MoveToZ), MoveToZ, MoveMinSpeed, MoveWorkSpeed); //�����ƶ���ָ�������
end;

procedure TMainForm.Button61MouseDown(Sender: TObject; Button: TMouseButton; Shift: TShiftState; X, Y: Integer); //Z������
begin
  MoveZ(TButton(Sender).Tag, StrToFloatDef(Trim(ZMoveStepEdit.Text), 1.0));
end;

procedure TMainForm.Button12MouseDown(Sender: TObject; Button: TMouseButton; Shift: TShiftState; X, Y: Integer);
begin
  MoveTimer.Tag := TButton(Sender).Tag;
  RunMove(False, True, MoveTimer.Tag, StrToFloatDef(Trim(XMoveStepEdit.Text), 1.0), StrToFloatDef(Trim(YMoveStepEdit.Text), 1.0)); //�ƶ���ָ��λ��
  CheckMouseTimer.Enabled := True; //������������ӳټ��
end;

procedure TMainForm.Button12MouseUp(Sender: TObject; Button: TMouseButton; Shift: TShiftState; X, Y: Integer);
begin   //�������ֹͣ���
  MoveTimer.Enabled := False;
  CheckMouseTimer.Enabled := False;
end;

procedure QuickMoveToPos(const X, Y: string); //�����ƶ���
var
  MoveToX, MoveToY: Double;
{
  cWorkAreaX: array[1..4] of Integer = (1, -1, -1, 1); //1-4����X����
  cWorkAreaY: array[1..4] of Integer = (1, 1, -1, -1); //1-4����Y����
}
begin
  MoveToX := StrToFloatDef(X, 50) * cWorkAreaX[WorkArea]; //
  MoveToY := StrToFloatDef(Y, 100) * cWorkAreaY[WorkArea]; //
  //�ӹ�ԭ�㶨�壺True:��еԭ��Ϊԭ�� False:��ǰ�����Ϊԭ��
  LPenQuickMoveTo(0, not MainForm.ZeroPointStyleCheckBox.Checked, (MoveToX), (MoveToY), 0, MoveMinSpeed, MoveWorkSpeed);  //XY False XZ True
end;

procedure TMainForm.Button3Click(Sender: TObject);  //�����ƶ���
begin
  case TButton(Sender).Tag of
    0:
      QuickMoveToPos(Trim(XMoveToPostEdit.Text), Trim(YMoveToPostEdit.Text)); //�����ƶ���
    1:
      QuickMoveToPos(Trim(XMoveToPostEdit1.Text), Trim(YMoveToPostEdit1.Text)); //�����ƶ���
    2:
      QuickMoveToPos(Trim(XMoveToPostEdit2.Text), Trim(YMoveToPostEdit2.Text)); //�����ƶ���
    3:
      QuickMoveToPos(Trim(XMoveToPostEdit3.Text), Trim(YMoveToPostEdit3.Text)); //�����ƶ���
  end;
end;

procedure TMainForm.MoveIntervalEditChange(Sender: TObject); //�����ƶ�Ƶ��
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
  RunMove(True, not ZeroPointStyleCheckBox.Checked, MoveTimer.Tag, StrToIntDef(Trim(XMoveStepEdit.Text), 1), StrToIntDef(Trim(YMoveStepEdit.Text), 1)); //�ƶ���ָ��λ��
end;

procedure TMainForm.CardIDtoServerBTNClick(Sender: TObject);
begin
  //Demo�У�
  //�а忨�����ɹ�������Ƿ�ע��
  //δע�ᣬ��ⰴť������
  //��ע�ᣬ��ⰴť����
  //�������⡱���ѵ�ǰ�忨���͡�ID��ע�����ύ����̨EXE
  //�ȴ���̨�����������ɹ���ʧ�ܣ�
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
  MoveTimer.Enabled := True; //�����Զ��ƶ�ָ���
end;

//�����ƶ����==================================================================================================

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

