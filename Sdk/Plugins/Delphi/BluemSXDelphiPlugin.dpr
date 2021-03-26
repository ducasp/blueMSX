library BluemSXDelphiPlugin;

uses
  Windows,
  sysutils,
  BlueMSXToolInterface in 'BlueMSXToolInterface.pas';

{$R *.res}

var
  toolSnapshotCreate              : TToolSnapshotCreate;
  toolSnapshotDestroy             : TToolSnapshotDestroy;
  toolGetState                    : TToolGetEmulatorState;
  toolSnapshotGetDeviceCount      : TToolSnapshotGetDeviceCount;
  toolSnapshotGetDevice           : TToolSnapshotGetDevice;

  toolDeviceGetMemoryBlockCount   : TToolDeviceGetMemoryBlockCount;
  toolDeviceGetMemoryBlock        : TToolDeviceGetMemoryBlock;
  toolDeviceGetRegisterBankCount  : TToolDeviceGetRegisterBankCount;
  toolDeviceGetRegisterBank       : TToolDeviceGetRegisterBank;
  toolDeviceGetIoPortsCount       : TToolDeviceGetIoPortsCount;
  toolDeviceGetIoPorts            : TToolDeviceGetIoPorts;
  toolDeviceRun                   : TToolAction;
  toolDeviceStop                  : TToolAction;
  toolDevicePause                 : TToolAction;
  toolDeviceStep                  : TToolAction;
  toolSetBreakpoint               : TToolBreakpoint;
  toolClearBreakpoint             : TToolBreakpoint;


// To be implemented by the user
procedure OnCreateTool;
begin
  MessageBox( 0, 'OnCreateTool', 'BlueMSX plugin', MB_OK or MB_APPLMODAL );
end;

procedure OnDestroyTool;
begin
  MessageBox( 0, 'OnDestroyTool', 'BlueMSX plugin', MB_OK or MB_APPLMODAL );
end;

procedure OnShowTool;
begin
  MessageBox( 0, 'OnShowTool', 'BlueMSX plugin', MB_OK or MB_APPLMODAL );
end;

procedure OnEmulatorStart;
begin
  MessageBox( 0, 'OnEmulatorStart', 'BlueMSX plugin', MB_OK or MB_APPLMODAL );
end;

procedure OnEmulatorStop;
begin
  MessageBox( 0, 'OnEmulatorStop', 'BlueMSX plugin', MB_OK or MB_APPLMODAL );
end;

procedure OnEmulatorPause;
begin
  MessageBox( 0, 'OnEmulatorPause', 'BlueMSX plugin', MB_OK or MB_APPLMODAL );
end;

procedure OnEmulatorResume;
begin
  MessageBox( 0, 'OnEmulatorResume', 'BlueMSX plugin', MB_OK or MB_APPLMODAL );
end;

function OnGetName: pchar;
begin
  Result := PChar ( 'Delphi BlueMSX Sample Plugin' );
end;

//------------------------------------------------------------------------------

function Create10 ( toolInterface: PInterface; name: pchar; length: integer ): integer; stdcall;
begin

  StrCopy( name, OnGetName );

  toolSnapshotCreate              := toolInterface^.create;
  toolSnapshotDestroy             := toolInterface^.destroy;
  toolGetState                    := toolInterface^.getState;
  toolSnapshotGetDeviceCount      := toolInterface^.getDeviceCount;
  toolSnapshotGetDevice           := toolInterface^.getDevice;
  toolDeviceGetMemoryBlockCount   := toolInterface^.getMemoryBlockCount;
  toolDeviceGetMemoryBlock        := toolInterface^.getMemoryBlock;
  toolDeviceGetRegisterBankCount  := toolInterface^.getRegisterBankCount;
  toolDeviceGetRegisterBank       := toolInterface^.getRegisterBank;
  toolDeviceGetIoPortsCount       := toolInterface^.getIoPortsCount;
  toolDeviceGetIoPorts            := toolInterface^.getIoPorts;
  toolDeviceRun                   := toolInterface^.run;
  toolDeviceStop                  := toolInterface^.stop;
  toolDevicePause                 := toolInterface^.pause;
  toolDeviceStep                  := toolInterface^.step;
  toolSetBreakpoint               := toolInterface^.setBreakpoint;
  toolClearBreakpoint             := toolInterface^.clearBreakpoint;
  OnCreateTool;
  Result := 1;
end;

procedure Destroy; stdcall;
begin
  OnDestroyTool;
end;

procedure Show; stdcall;
begin
  OnShowTool;
end;

procedure NotifyEmulatorStart; stdcall;
begin
  OnEmulatorStart;
end;

procedure NotifyEmulatorStop; stdcall;
begin
  OnEmulatorStop;
end;

procedure NotifyEmulatorPause; stdcall;
begin
  OnEmulatorPause;
end;

procedure NotifyEmulatorResume; stdcall;
begin
  OnEmulatorResume;
end;

exports
  Create10 index 1,
  Destroy index 2,
  Show index 3,
  NotifyEmulatorStart index 4,
  NotifyEmulatorStop index 5,
  NotifyEmulatorPause index 6,
  NotifyEmulatorResume index 7;

begin
end.
