/******************************************************************************
*
* Main Dev Kit Application
* Revision 1.0
*
* Notes:
* Initial Software package that is developed for the Dev Kit.  07/06/16 - EBJ
*
******************************************************************************/

#include "LApp.h"

//////////////////////////// MAIN //////////////////// MAIN //////////////
int main()
{
	// Initialize System
    init_platform();  		// This initializes the platform, which is ...
	ps7_post_config();
	Xil_DCacheDisable();	//
	InitializeAXIDma();		// Initialize the AXI DMA Transfer Interface
	Xil_Out32 (XPAR_AXI_GPIO_16_BASEADDR, 16384);
	Xil_Out32 (XPAR_AXI_GPIO_17_BASEADDR , 1);
	InitializeInterruptSystem(XPAR_PS7_SCUGIC_0_DEVICE_ID);

	for (i=0; i<32; i++ ) { RecvBuffer[i] = '_'; }		// Clear RecvBuffer Variable
	for (i=0; i<32; i++ ) { SendBuffer[i] = '_'; }		// Clear SendBuffer Variable

	//*******************Setup the UART **********************//
	XUartPs_Config *Config = XUartPs_LookupConfig(UART_DEVICEID);
	if (NULL == Config) { return 1;}
	Status = XUartPs_CfgInitialize(&Uart_PS, Config, Config->BaseAddress);
	if (Status != 0){ return 1;	}

	/* Conduct a Selftest for the UART */
	Status = XUartPs_SelfTest(&Uart_PS);
	if (Status != 0) { return 1; }

	/* Set to normal mode. */
	XUartPs_SetOperMode(&Uart_PS, XUARTPS_OPER_MODE_NORMAL);
	//*******************Setup the UART **********************//

	//*******************Receive and Process Packets **********************//
	Xil_Out32 (XPAR_AXI_GPIO_0_BASEADDR, 11);
	Xil_Out32 (XPAR_AXI_GPIO_1_BASEADDR, 71);
	Xil_Out32 (XPAR_AXI_GPIO_2_BASEADDR, 167);
	Xil_Out32 (XPAR_AXI_GPIO_3_BASEADDR, 2015);
/*	Xil_Out32 (XPAR_AXI_GPIO_4_BASEADDR, 12);
	Xil_Out32 (XPAR_AXI_GPIO_5_BASEADDR, 75);
	Xil_Out32 (XPAR_AXI_GPIO_6_BASEADDR, 75);
	Xil_Out32 (XPAR_AXI_GPIO_7_BASEADDR, 5);
	Xil_Out32 (XPAR_AXI_GPIO_8_BASEADDR, 25);*/
	//*******************enable the system **********************//
	Xil_Out32(XPAR_AXI_GPIO_18_BASEADDR, 1);
	//*******************Receive and Process Packets **********************//

	//******************Setup Detector and Module Objects*****************//
	//LDetector *Detector = LDetector();
	//Detector->SetMode(1); // Processed Data Mode
	//******************Setup Detector and Module Objects*****************//


	// *********** Setup the Hardware Reset GPIO ****************//
	GPIOConfigPtr = XGpioPs_LookupConfig(XPAR_PS7_GPIO_0_DEVICE_ID);
	Status = XGpioPs_CfgInitialize(&Gpio, GPIOConfigPtr, GPIOConfigPtr ->BaseAddr);
	if (Status != XST_SUCCESS) { return XST_FAILURE; }
	XGpioPs_SetDirectionPin(&Gpio, SW_BREAK_GPIO, 1);
	// *********** Setup the Hardware Reset MIO ****************//

	// *********** Mount SD Card and Initialize Variables ****************//
	if( doMount == 0 ){			//Initialize the SD card here
		ffs_res = f_mount(0, NULL);
		ffs_res = f_mount(0, &fatfs);
		doMount = 1;
	}
	if( f_stat( cLogFile, &fno) ){	// f_stat returns non-zero if not open, so open the file
		char cZeroBuffer[] = "0000000000 ";
		ffs_res = f_open(&logFile, cLogFile, FA_WRITE|FA_OPEN_ALWAYS);
		ffs_res = f_write(&logFile, cZeroBuffer, 10, &numBytesWritten);
		filptr_clogFile += 10;		// Protect the first xx number of bytes to use as flags
		ffs_res = f_close(&logFile);
	}
	else // If the file exists, read it
	{

		ffs_res = f_open(&logFile, cLogFile, FA_READ|FA_WRITE);	//open with read/write access
		ffs_res = f_lseek(&logFile, 0);					//go to beginning of file
		ffs_res = f_read(&logFile, &filptr_buffer, 10, &numBytesRead);	//Read the first xx bytes to determine flags and the size of the file pointer
		sscanf(filptr_buffer, "%d", &filptr_clogFile);
		iSprintfReturn = snprintf(cWriteToLogFile, LOG_FILE_BUFF_SIZE, "POWER RESET %f ", dTime);
		ffs_res = f_write(&logFile, cWriteToLogFile, iSprintfReturn, &numBytesWritten);
		filptr_clogFile += numBytesWritten;
		ffs_res = f_close(&logFile);
	}
	// *********** Mount SD Card and Initialize Variables ****************//

	// ******************* POLLING LOOP *******************//
	xil_printf("\n\r Turn on Local Echo: under Terminal-Setup in Tera Term \n\r");
	xil_printf(" Code is expecting a 'Return' after Each Command \n\r");
	while(1){
		sw = 0;   //  stop switch reset to 0
		XUartPs_SetOptions(&Uart_PS,XUARTPS_OPTION_RESET_RX);	// Clear UART Read Buffer
		for (i=0; i<32; i++ ) { RecvBuffer[i] = '_'; }			// Clear RecvBuffer Variable

		sleep(0.5);  // Built in Latency ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ 0.5 s
		xil_printf("\n\r Devkit version 2.23 \n\r");
		xil_printf("MAIN MENU \n\r");
		xil_printf("*****\n\r");
		xil_printf(" 0) Set Mode of Operation\n\r");
		xil_printf(" 1) Enable or disable the system\n\r");
		xil_printf(" 2) Continuously Read of Processed Data\n\r");
		xil_printf("\n\r **Setup Parameters ** \n\r");
		xil_printf(" 3) Set Trigger Threshold\n\r");
		xil_printf(" 4) Set Integration Times (number of clock cycles * 4ns) \n\r");
		xil_printf("\n\r ** Additional Commands ** \n\r");
		xil_printf(" 5) Perform a DMA transfer of Waveform Data\n\r");
		xil_printf(" 6) Perform a DMA transfer of Processed Data\n\r");
		xil_printf(" 7) Check the Size of the Data Buffered (Max = 4095) \n\r");
		xil_printf(" 8) Clear the Processed Data Buffers\n\r");
		xil_printf(" 9) Execute Print of Data on DRAM \n\r");
		xil_printf("10) GUI Serial Transfer \n\r");
		xil_printf("11) GUI Serial Change Trigger Threshold\n\r");
		xil_printf("12) GUI Serial Change Integration Times\n\r");
		xil_printf("13) GUI Transfer Processed Data\n\r");
		xil_printf("14) High Voltage and Temperature Control \n\r");
		xil_printf("******\n\r");
		while (XUartPs_IsSending(&Uart_PS)) { }  // Wait until Write Buffer is Sent

		ReadCommandPoll();
		menusel = 99999;
		sscanf(RecvBuffer,"%02d",&menusel);
		if ( menusel < 0 || menusel > 14 ) {
			xil_printf(" Invalid Command: Enter 0-14 \n\r");
			sleep(1); 			// Built in Latency ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ 1 s
		}

		switch (menusel) { // Switch-Case Menu Select

		case 0: //Set Mode of Operation
			mode = 99; //Detector->GetMode();
			xil_printf("\n\r Waveform Data: \t Enter 0 <return>\n\r");
			xil_printf(" LPF Waveform Data: \t Enter 1 <return>\n\r");
			xil_printf(" DFF Waveform Data: \t Enter 2 <return>\n\r");
			xil_printf(" TRG Waveform Data: \t Enter 3 <return>\n\r");
			xil_printf(" Processed Data: \t Enter 4 <return>\n\r");
			ReadCommandPoll();
			sscanf(RecvBuffer,"%01d",&mode);

			if (mode < 0 || mode > 4 ) { xil_printf("Invalid Command\n\r"); break; }
			Xil_Out32 (XPAR_AXI_GPIO_14_BASEADDR, ((u32)mode));
			// Register 14
			if ( mode == 0 ) { xil_printf("Transfer AA Waveforms\n\r"); }
			if ( mode == 1 ) { xil_printf("Transfer LPF Waveforms\n\r"); }
			if ( mode == 2 ) { xil_printf("Transfer DFF Waveforms\n\r"); }
			if ( mode == 3 ) { xil_printf("Transfer TRG Waveforms\n\r"); }
			if ( mode == 4 ) { xil_printf("Transfer Processed Data\n\r"); }
			sleep(1); 			// Built in Latency ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ 1 s

			dTime += 1;
			iSprintfReturn = snprintf(cWriteToLogFile, LOG_FILE_BUFF_SIZE, "Set mode %d %f ", mode, dTime);

			ffs_res = f_open(&logFile, cLogFile, FA_OPEN_ALWAYS | FA_WRITE);
			if(ffs_res){
				xil_printf("Could not open file %d", ffs_res);
				break;
			}
			ffs_res = f_lseek(&logFile, filptr_clogFile);
			ffs_res = f_write(&logFile, cWriteToLogFile, iSprintfReturn, &numBytesWritten);
			filptr_clogFile += numBytesWritten;
			snprintf(cWriteToLogFile, 10, "%d", filptr_clogFile);
			ffs_res = f_lseek(&logFile, (10 - LNumDigits(filptr_clogFile)));	// Move to the start of the file
			ffs_res = f_write(&logFile, cWriteToLogFile, LNumDigits(filptr_clogFile), &numBytesWritten);	//Record the new file pointer
			ffs_res = f_close(&logFile);
			break;

		case 1: //Enable or disable the system
			xil_printf("\n\r Disable: Enter 0 <return>\n\r");
			xil_printf(" Enable: Enter 1 <return>\n\r");
			ReadCommandPoll();
			sscanf(RecvBuffer,"%01d",&enable_state);
			if (enable_state != 0 && enable_state != 1) { xil_printf("Invalid Command\n\r"); break; }
			Xil_Out32(XPAR_AXI_GPIO_18_BASEADDR, ((u32)enable_state));
			// Register 18 Out enabled, In Disabled
			if ( enable_state == 1 ) { xil_printf("DAQ Enabled\n\r"); }
			if ( enable_state == 0 ) { xil_printf("DAQ Disabled\n\r"); }
			sleep(1); 			// Built in Latency ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ 1 s

			dTime += 1;
			iSprintfReturn = snprintf(cWriteToLogFile, LOG_FILE_BUFF_SIZE, "Enable system %d %f ", enable_state, dTime);

			ffs_res = f_open(&logFile, cLogFile, FA_OPEN_ALWAYS | FA_WRITE);
			if(ffs_res){
				xil_printf("Could not open file %d", ffs_res);
				break;
			}
			ffs_res = f_lseek(&logFile, filptr_clogFile);
			ffs_res = f_write(&logFile, cWriteToLogFile, iSprintfReturn, &numBytesWritten);
			filptr_clogFile += numBytesWritten;												// Add the number of bytes written to the file pointer
			snprintf(cWriteToLogFile, 10, "%d", filptr_clogFile);							// Write that to a string for saving
			ffs_res = f_lseek(&logFile, (10 - LNumDigits(filptr_clogFile)));				// Seek to the beginning of the file skipping the leading zeroes
			ffs_res = f_write(&logFile, cWriteToLogFile, LNumDigits(filptr_clogFile), &numBytesWritten); // Write the new file pointer
			ffs_res = f_close(&logFile);

			break;

		case 2: //Continuously Read of Processed Data

			dTime += 1;
			iSprintfReturn = snprintf(cWriteToLogFile, LOG_FILE_BUFF_SIZE, "Print data %d %f ", enable_state, dTime);

			ffs_res = f_open(&logFile, cLogFile, FA_OPEN_ALWAYS | FA_WRITE);
			if(ffs_res){
				xil_printf("Could not open file %d", ffs_res);
				break;
			}
			ffs_res = f_lseek(&logFile, filptr_clogFile);
			ffs_res = f_write(&logFile, cWriteToLogFile, iSprintfReturn, &numBytesWritten);
			filptr_clogFile += numBytesWritten;
			snprintf(cWriteToLogFile, 10, "%d", filptr_clogFile);							// Write that to a string for saving
			ffs_res = f_lseek(&logFile, (10 - LNumDigits(filptr_clogFile)));				// Seek to the beginning of the file skipping the leading zeroes
			ffs_res = f_write(&logFile, cWriteToLogFile, LNumDigits(filptr_clogFile), &numBytesWritten); // Write the new file pointer
			ffs_res = f_close(&logFile);

			xil_printf("\n\r ********Data Acquisition:\n\r");
			xil_printf(" Press 'q' to Stop or Press Hardware USR reset button  \n\r");
			xil_printf(" Press <return> to Start");
			ReadCommandPoll();	//Just get a '\n' to continue
			DAQ();
			sw = 0;   // broke out of the read loop, stop switch reset to 0
			break;

		case 3: //Set Threshold
			iThreshold0 = Xil_In32(XPAR_AXI_GPIO_10_BASEADDR);
			xil_printf("\n\r Existing Threshold = %d \n\r",Xil_In32(XPAR_AXI_GPIO_10_BASEADDR));
			xil_printf(" Enter Threshold (6144 to 10240) <return> \n\r");
			ReadCommandPoll();
			sscanf(RecvBuffer,"%04d",&iThreshold1);
			Xil_Out32(XPAR_AXI_GPIO_10_BASEADDR, ((u32)iThreshold1));
			xil_printf("New Threshold = %d \n\r",Xil_In32(XPAR_AXI_GPIO_10_BASEADDR));
			sleep(1); 			// Built in Latency ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ 1 s

			dTime += 1;
			iSprintfReturn = snprintf(cWriteToLogFile, LOG_FILE_BUFF_SIZE, "Set trigger threshold from %d to %d %f ", iThreshold0, iThreshold1, dTime);

			ffs_res = f_open(&logFile, cLogFile, FA_OPEN_ALWAYS | FA_WRITE);
			if(ffs_res){
				xil_printf("Could not open file %d", ffs_res);
				break;
			}
			ffs_res = f_lseek(&logFile, filptr_clogFile);
			ffs_res = f_write(&logFile, cWriteToLogFile, iSprintfReturn, &numBytesWritten);
			filptr_clogFile += numBytesWritten;
			snprintf(cWriteToLogFile, 10, "%d", filptr_clogFile);							// Write that to a string for saving
			ffs_res = f_lseek(&logFile, (10 - LNumDigits(filptr_clogFile)));				// Seek to the beginning of the file skipping the leading zeroes
			ffs_res = f_write(&logFile, cWriteToLogFile, LNumDigits(filptr_clogFile), &numBytesWritten); // Write the new file pointer
			ffs_res = f_close(&logFile);
			break;

		case 4: //Set Integration Times
			setIntsArray[4] = -52 + ((int)Xil_In32(XPAR_AXI_GPIO_0_BASEADDR))*4;
			setIntsArray[5] = -52 + ((int)Xil_In32(XPAR_AXI_GPIO_1_BASEADDR))*4;
			setIntsArray[6] = -52 + ((int)Xil_In32(XPAR_AXI_GPIO_2_BASEADDR))*4;
			setIntsArray[7] = -52 + ((int)Xil_In32(XPAR_AXI_GPIO_3_BASEADDR))*4;

			xil_printf("\n\r Existing Integration Times \n\r");
			xil_printf(" Time = 0 ns is when the Pulse Crossed Threshold \n\r");
			xil_printf(" Baseline Integral Window \t [-200ns,%dns] \n\r",-52 + ((int)Xil_In32(XPAR_AXI_GPIO_0_BASEADDR))*4 );
			xil_printf(" Short Integral Window \t [-200ns,%dns] \n\r",-52 + ((int)Xil_In32(XPAR_AXI_GPIO_1_BASEADDR))*4 );
			xil_printf(" Long Integral Window  \t [-200ns,%dns] \n\r",-52 + ((int)Xil_In32(XPAR_AXI_GPIO_2_BASEADDR))*4 );
			xil_printf(" Full Integral Window  \t [-200ns,%dns] \n\r",-52 + ((int)Xil_In32(XPAR_AXI_GPIO_3_BASEADDR))*4 );
			xil_printf(" Change: (Y)es (N)o <return>\n\r");
			ReadCommandPoll();
			sscanf(RecvBuffer,"%c",&updateint);

			if (updateint == 'N' || updateint == 'n') {
				dTime += 1;
				iSprintfReturn = snprintf(cWriteToLogFile, LOG_FILE_BUFF_SIZE, "No change to integration times of %d %d %d %d %f ", setIntsArray[4], setIntsArray[5], setIntsArray[6], setIntsArray[7], dTime);
				ffs_res = f_open(&logFile, cLogFile, FA_OPEN_ALWAYS | FA_WRITE);
				if(ffs_res){
					xil_printf("Could not open file %d", ffs_res);
					break;
				}
				ffs_res = f_lseek(&logFile, filptr_clogFile);
				ffs_res = f_write(&logFile, cWriteToLogFile, iSprintfReturn, &numBytesWritten);
				filptr_clogFile += numBytesWritten;
				snprintf(cWriteToLogFile, 10, "%d", filptr_clogFile);							// Write that to a string for saving
				ffs_res = f_lseek(&logFile, (10 - LNumDigits(filptr_clogFile)));				// Seek to the beginning of the file skipping the leading zeroes
				ffs_res = f_write(&logFile, cWriteToLogFile, LNumDigits(filptr_clogFile), &numBytesWritten); // Write the new file pointer
				ffs_res = f_close(&logFile);
				break;
			}

			if (updateint == 'Y' || updateint == 'y') {
				SetIntegrationTimes(&setIntsArray, &setSamples);
				dTime += 1;
				iSprintfReturn = snprintf(cWriteToLogFile, LOG_FILE_BUFF_SIZE, "Set integration times to %d %d %d %d from %d %d %d %d %f ",
						setIntsArray[0], setIntsArray[1], setIntsArray[2], setIntsArray[3], setIntsArray[4], setIntsArray[5], setIntsArray[6], setIntsArray[7], dTime);
				ffs_res = f_open(&logFile, cLogFile, FA_OPEN_ALWAYS | FA_WRITE);
				if(ffs_res){
					xil_printf("Could not open file %d", ffs_res);
					break;
				}
				ffs_res = f_lseek(&logFile, filptr_clogFile);
				ffs_res = f_write(&logFile, cWriteToLogFile, iSprintfReturn, &numBytesWritten);
				filptr_clogFile += numBytesWritten;
				snprintf(cWriteToLogFile, 10, "%d", filptr_clogFile);							// Write that to a string for saving
				ffs_res = f_lseek(&logFile, (10 - LNumDigits(filptr_clogFile)));				// Seek to the beginning of the file skipping the leading zeroes
				ffs_res = f_write(&logFile, cWriteToLogFile, LNumDigits(filptr_clogFile), &numBytesWritten); // Write the new file pointer
				ffs_res = f_close(&logFile);

			}

			sleep(1); 			// Built in Latency ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ 1 s
			break;

		case 5: //Perform a DMA transfer
			xil_printf("\n\r Perform DMA Transfer of Waveform Data\n\r");
			xil_printf(" Press 'q' to Exit Transfer  \n\r");
			Xil_Out32 (XPAR_AXI_DMA_0_BASEADDR + 0x48, 0xa000000);
			Xil_Out32 (XPAR_AXI_DMA_0_BASEADDR + 0x58 , 65536);
			sleep(1); 			// Built in Latency ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ 1 s
			PrintData();		// Display data to console.
			sw = 0;   // broke out of the read loop, stop swith reset to 0
			break;

		case 6: //Perform a DMA transfer of Processed data
			xil_printf("\n\r ********Perform DMA Transfer of Processed Data \n\r");
			xil_printf(" Press 'q' to Exit Transfer  \n\r");
			//Xil_Out32 (XPAR_AXI_GPIO_18_BASEADDR, 0);	// Disable : GPIO Reg Capture Module
			Xil_Out32 (XPAR_AXI_GPIO_15_BASEADDR, 1);	// Enable: GPIO Reg to Readout Data MUX
			//sleep(1);				// Built in Latency ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ 1 s
			Xil_Out32 (XPAR_AXI_DMA_0_BASEADDR + 0x48, 0xa000000); // Transfer from BRAM to DRAM, start address 0xa000000, 16-bit length
			Xil_Out32 (XPAR_AXI_DMA_0_BASEADDR + 0x58 , 65536);
			sleep(1); 			// Built in Latency ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ 1 s
			Xil_Out32 (XPAR_AXI_GPIO_15_BASEADDR, 0); 	// Disable: GPIO Reg turn off Readout Data MUX
			ClearBuffers();
			PrintData();								// Display data to console.
			//Xil_Out32 (XPAR_AXI_GPIO_18_BASEADDR, 1);	// Enable : GPIO Reg Capture Module
			sw = 0;   // broke out of the read loop, stop swith reset to 0
			sleep(1); 			// Built in Latency ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ 1 s
			break;

		case 7: //Check the Size of the Data Buffers
			databuff = Xil_In32 (XPAR_AXI_GPIO_11_BASEADDR);
			xil_printf("\n\r BRAM Data Buffer Size = %d \n\r",databuff);
			sleep(1); 			// Built in Latency ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ 1 s
			break;

		case 8: //Clear the processed data buffers
			xil_printf("\n\r Clear the Data Buffers\n\r");
			ClearBuffers();
			sleep(1); 			// Built in Latency ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ 1 s
			break;

		case 9: //Print DMA Data
			xil_printf("\n\r Print Data\n\r");
			PrintData();
			break;
		case 10: //Transfer data over serial port	// Skips setting integrals, threshold	//Difference is this doesn't save to sd, it prints out the data for us
			//xil_printf("321");	//echo back to the gui that the request was received
			mode = 0;
			Xil_Out32 (XPAR_AXI_GPIO_14_BASEADDR, ((u32)mode));	//sets mode to transfer AA waveforms
			sleep(1);
			enable_state = 1;
			Xil_Out32(XPAR_AXI_GPIO_18_BASEADDR, ((u32)enable_state));	//enables the system
			sleep(1);
			getWFDAQ();
			break;
		case 11: //change threshold over the serial connection
			xil_printf("Enter the new threshold: <enter>\n");
			ReadCommandPoll();
			sscanf(RecvBuffer,"%04d",&iThreshold1);
			Xil_Out32(XPAR_AXI_GPIO_10_BASEADDR, ((u32)iThreshold1));
			xil_printf("New Threshold = %d \n\r",Xil_In32(XPAR_AXI_GPIO_10_BASEADDR));
			sleep(1); 			// Built in Latency ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ 1 s
			break;
		case 12: //change integrals over the serial connection
			xil_printf("Enter each integral time followed by <enter>");
			ReadCommandPoll();
			sscanf(RecvBuffer,"%04d",&setBL);
			ReadCommandPoll();
			sscanf(RecvBuffer,"%04d",&setSI);
			ReadCommandPoll();
			sscanf(RecvBuffer,"%04d",&setLI);
			ReadCommandPoll();
			sscanf(RecvBuffer,"%04d",&setFI);

			Xil_Out32 (XPAR_AXI_GPIO_0_BASEADDR, ((u32)((setBL+52)/4)));	//set baseline int time
			Xil_Out32 (XPAR_AXI_GPIO_1_BASEADDR, ((u32)((setSI+52)/4)));	//set short int time
			Xil_Out32 (XPAR_AXI_GPIO_2_BASEADDR, ((u32)((setLI+52)/4)));	//set long int time
			Xil_Out32 (XPAR_AXI_GPIO_3_BASEADDR, ((u32)((setFI+52)/4)));	//set full int time

			//echo back the changed values //for checking
			xil_printf("  Inputs Rounded to the Nearest 4 ns : Number of Samples\n\r");
			xil_printf("  Baseline Integral Window  [-200ns,%dns]: %d \n\r",-52 + ((int)Xil_In32(XPAR_AXI_GPIO_0_BASEADDR))*4, 38+Xil_In32(XPAR_AXI_GPIO_0_BASEADDR) );
			xil_printf("  Short Integral Window 	  [-200ns,%dns]: %d \n\r",-52 + ((int)Xil_In32(XPAR_AXI_GPIO_1_BASEADDR))*4, 38+Xil_In32(XPAR_AXI_GPIO_1_BASEADDR));
			xil_printf("  Long Integral Window      [-200ns,%dns]: %d \n\r",-52 + ((int)Xil_In32(XPAR_AXI_GPIO_2_BASEADDR))*4, 38+Xil_In32(XPAR_AXI_GPIO_2_BASEADDR));
			xil_printf("  Full Integral Window      [-200ns,%dns]: %d \n\r",-52 + ((int)Xil_In32(XPAR_AXI_GPIO_3_BASEADDR))*4, 38+Xil_In32(XPAR_AXI_GPIO_3_BASEADDR));
			break;
		case 13: //Transfer processed data over the serial port
			mode = 4;
			Xil_Out32 (XPAR_AXI_GPIO_14_BASEADDR, ((u32)mode));	//sets mode to processed data
			sleep(1);
			enable_state = 1;
			Xil_Out32(XPAR_AXI_GPIO_18_BASEADDR, ((u32)enable_state)); //enables the system
			sleep(1);
			getWFDAQ();
			break;
		case 14: //HV and temp control
			xil_printf("************HIGH VOLTAGE CONTROL************\n\r");
			xil_printf(" 0) Write value to potentiometer\n\r");
			xil_printf(" 1) Read value from potentiometer\n\r");
			xil_printf(" 2) Store value to EEPROM\n\r");
			xil_printf(" 3) Read value from EEPROM\n\r");
			xil_printf("*************TEMPERATURE SENSOR*************\n\r");
			xil_printf(" 4) Read temperature\n\r");

			ReadCommandPoll();
			menusel = 99999;
			sscanf(RecvBuffer,"%01d",&menusel);
			if( menusel < 0 || menusel > 4) { xil_printf("Invalid command.\n\r"); sleep(1); continue; }

			switch(menusel){
			case 0:	//Write Pots
				IIC_SLAVE_ADDR=&IIC_SLAVE_ADDR1;
				cntrl = 0x1; //command 1 - write contents of serial register data to RDAC - see AD5144 datasheet pg 26
				xil_printf(" Enter a value from 1-256 taps to write to the potentiometer  \n\r");
				ReadCommandPoll();
				sscanf(RecvBuffer,"%d",&data_bits);
				if(data_bits < 1 || data_bits > 256) { xil_printf("Invalid number of taps.\n\r"); sleep(1); continue; }
				xil_printf(" Enter a number 1-4 to select a particular potentiometer to adjust or 5 for all potentiometers  \n\r");
				ReadCommandPoll();
				sscanf(RecvBuffer,"%01d",&rdac);
				if(rdac < 1 || rdac > 5) { xil_printf("Invalid pot selection.\n\r"); sleep(1); continue; }
				xil_printf("%d \t %d \n\r", data_bits, rdac);

				switch(rdac){
				case 1:
					pot_addr = 0x0;
					break;
				case 2:
					pot_addr = 0x1;
					break;
				case 3:
					pot_addr = 0x2;
					break;
				case 4:
					pot_addr = 0x3;
					break;
				case 5:
					pot_addr = 0x8;
					break;
				default:
					xil_printf("Invalid. No changes made.");
					break;
				}

				a = cntrl<<4|pot_addr;
				i2c_Send_Buffer[0] = a;
				i2c_Send_Buffer[1] = data_bits;
				Status = IicPsMasterSend(IIC_DEVICE_ID, i2c_Send_Buffer, i2c_Recv_Buffer, IIC_SLAVE_ADDR);
				break;
			case 1:	//Read Pots
				IIC_SLAVE_ADDR=&IIC_SLAVE_ADDR1;
				cntrl = 0x3; //command 3 - read back contents - see AD5144 datasheet pg 26
				xil_printf(" Enter a number 1-4 to select which potentiometer to read value  \n\r");
				ReadCommandPoll();
				sscanf(RecvBuffer,"%01d",&rdac);
				if(rdac < 1 || rdac > 4) { xil_printf("Invalid pot selection.\n\r"); sleep(1); continue; }

				switch(rdac){
				case 1:
					pot_addr = 0x0;
					break;
				case 2:
					pot_addr = 0x1;
					break;
				case 3:
					pot_addr = 0x2;
					break;
				case 4:
					pot_addr = 0x3;
					break;
				default:
					xil_printf("Invalid. No changes made.");
					break;
				}

				a = cntrl<<4|pot_addr;
				i2c_Send_Buffer[0] = a;
				i2c_Send_Buffer[1] = 0x3;
				Status = IicPsMasterSend(IIC_DEVICE_ID, i2c_Send_Buffer, i2c_Recv_Buffer, IIC_SLAVE_ADDR);
				Status = IicPsMasterRecieve(IIC_DEVICE_ID, i2c_Recv_Buffer, IIC_SLAVE_ADDR);
				xil_printf("%d \t", i2c_Recv_Buffer[0]);
				xil_printf("taps\n");
				break;
			case 2:	//Write EEPROM
				IIC_SLAVE_ADDR=&IIC_SLAVE_ADDR1;
				cntrl = 0x7; //command 9 - Copy RDAC register to EEPROM - see AD5144 datasheet pg 26
				xil_printf("Select which potentiometer value to store in EEPROM (1-4)   \n\r");
				ReadCommandPoll();
				sscanf(RecvBuffer,"%01d",&rdac);
				if(rdac < 1 || rdac > 4) { xil_printf("Invalid pot selection.\n\r"); sleep(1); continue; }
				xil_printf("%d \t %d \n\r", data_bits, rdac);

				switch(rdac){
				case 1:
					pot_addr = 0x0;
					break;
				case 2:
					pot_addr = 0x1;
					break;
				case 3:
					pot_addr = 0x2;
					break;
				case 4:
					pot_addr = 0x3;
					break;
				case 5:
					pot_addr = 0x8;
					break;
				default:
					xil_printf("Invalid. No changes made.");
					break;
				}

				a = cntrl<<4|pot_addr;
				i2c_Send_Buffer[0] = a;
				i2c_Send_Buffer[1] = 0x1;
				Status = IicPsMasterSend(IIC_DEVICE_ID, i2c_Send_Buffer, i2c_Recv_Buffer, IIC_SLAVE_ADDR);
				break;
			case 3:	//Read EEPROM
				IIC_SLAVE_ADDR=&IIC_SLAVE_ADDR1;
				cntrl = 0x3;//command 3 - Read back contents - see AD5144 datasheet pg 26
				xil_printf(" Enter a number 1-4 to select which EEPROM to read  \n\r");
				ReadCommandPoll();
				sscanf(RecvBuffer,"%01d",&rdac);
				if(rdac < 1 || rdac > 4) { xil_printf("Invalid pot selection.\n\r"); sleep(1); continue; }

				switch(rdac){
				case 1:
					pot_addr = 0x0;
					break;
				case 2:
					pot_addr = 0x1;
					break;
				case 3:
					pot_addr = 0x2;
					break;
				case 4:
					pot_addr = 0x3;
					break;
				default:
					xil_printf("Invalid. No changes made.");
					break;
				}

				//data_bits = 0x3;
				a = cntrl<<4|pot_addr;
				i2c_Send_Buffer[0] = a;
				i2c_Send_Buffer[1] = 0x1;
				Status = IicPsMasterSend(IIC_DEVICE_ID, i2c_Send_Buffer, i2c_Recv_Buffer, IIC_SLAVE_ADDR);
				Status = IicPsMasterRecieve(IIC_DEVICE_ID, i2c_Recv_Buffer, IIC_SLAVE_ADDR);
				xil_printf("%d \t", i2c_Recv_Buffer[0]);
				xil_printf("taps\n");
				break;
			case 4:	//Read Temp
				IIC_SLAVE_ADDR=&IIC_SLAVE_ADDR2;
				i2c_Send_Buffer[0] = 0x0;
				i2c_Send_Buffer[1] = 0x0;
				Status = IicPsMasterSend(IIC_DEVICE_ID, i2c_Send_Buffer, i2c_Recv_Buffer, IIC_SLAVE_ADDR);
				Status = IicPsMasterRecieve(IIC_DEVICE_ID, i2c_Recv_Buffer, IIC_SLAVE_ADDR);
				a = i2c_Recv_Buffer[0]<< 5;
				b = a | i2c_Recv_Buffer[1] >> 3;
				b /= 16;
				xil_printf("%d\xf8\x43\n", b);
				break;
			default:
				i = 1;
				break;
			}//end of case statement


			break;
		default :
			break;
		} // End Switch-Case Menu Select

	}	// ******************* POLLING LOOP *******************//


    cleanup_platform();  // Clean up the platform, which is ...

    return 0;
}
//////////////////////////// MAIN //////////////////// MAIN //////////////

//////////////////////////// InitializeAXIDma////////////////////////////////
// Sets up the AXI DMA
int InitializeAXIDma(void) {
	u32 tmpVal_0 = 0;
	u32 tmpVal_1 = 0;

	tmpVal_0 = Xil_In32(XPAR_AXI_DMA_0_BASEADDR + 0x30);

	tmpVal_0 = tmpVal_0 | 0x1001; //<allow DMA to produce interrupts> 0 0 <run/stop>

	Xil_Out32 (XPAR_AXI_DMA_0_BASEADDR + 0x30, tmpVal_0);
	tmpVal_1 = Xil_In32(XPAR_AXI_DMA_0_BASEADDR + 0x30);

	return 0;
}
//////////////////////////// InitializeAXIDma////////////////////////////////

//////////////////////////// InitializeInterruptSystem////////////////////////////////
int InitializeInterruptSystem(u16 deviceID) {
	int Status;

	GicConfig = XScuGic_LookupConfig (deviceID);

	if(NULL == GicConfig) {

		return XST_FAILURE;
	}

	Status = XScuGic_CfgInitialize(&InterruptController, GicConfig, GicConfig->CpuBaseAddress);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;

	}

	Status = SetUpInterruptSystem(&InterruptController);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;

	}

	Status = XScuGic_Connect (&InterruptController,
			XPAR_FABRIC_AXI_DMA_0_S2MM_INTROUT_INTR,
			(Xil_ExceptionHandler) InterruptHandler, NULL);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;

	}

	XScuGic_Enable(&InterruptController, XPAR_FABRIC_AXI_DMA_0_S2MM_INTROUT_INTR );

	return XST_SUCCESS;

}
//////////////////////////// InitializeInterruptSystem////////////////////////////////


//////////////////////////// Interrupt Handler////////////////////////////////
void InterruptHandler (void ) {

	u32 tmpValue;
	tmpValue = Xil_In32(XPAR_AXI_DMA_0_BASEADDR + 0x34);
	tmpValue = tmpValue | 0x1000;
	Xil_Out32 (XPAR_AXI_DMA_0_BASEADDR + 0x34, tmpValue);

	global_frame_counter++;
}
//////////////////////////// Interrupt Handler////////////////////////////////


//////////////////////////// SetUp Interrupt System////////////////////////////////
int SetUpInterruptSystem(XScuGic *XScuGicInstancePtr) {
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT, (Xil_ExceptionHandler)XScuGic_InterruptHandler, XScuGicInstancePtr);
	Xil_ExceptionEnable();
	return XST_SUCCESS;

}
//////////////////////////// SetUp Interrupt System////////////////////////////////

//////////////////////////// ReadCommandPoll////////////////////////////////
// Function used to clear the read buffer
// Read In new command, expecting a <return>
// Returns buffer size read
int ReadCommandPoll() {
	u32 rbuff = 0;			// read buffer size returned
	int i = 0; 				// index
	XUartPs_SetOptions(&Uart_PS,XUARTPS_OPTION_RESET_RX);	// Clear UART Read Buffer
	for (i=0; i<32; i++ ) { RecvBuffer[i] = '_'; }			// Clear RecvBuffer Variable
	while (!(RecvBuffer[rbuff-1] == '\n' || RecvBuffer[rbuff-1] == '\r' || RecvBuffer[rbuff-1] == 'd')) {
		rbuff += XUartPs_Recv(&Uart_PS, &RecvBuffer[rbuff],(32 - rbuff));
		sleep(0.1);			// Built in Latency ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ 0.1 s
	}
	return rbuff;

}
//////////////////////////// ReadCommandPoll////////////////////////////////


//////////////////////////// Set Integration Times ////////////////////////////////
// wfid = 	0 for AA
//			1 for DFF
//			2 for LPF
// At the moment, the software is expecting a 5 char buffer.  This needs to be fixed.
void SetIntegrationTimes(int * setIntsArray, u32 * setSamples){
	//int setIntsArray[8] = {};
	//u32 setsamples[8] = {};

	xil_printf("  Enter Baseline Stop Time in ns: -52 to 0 <return> \t");
	ReadCommandPoll();
	sscanf(RecvBuffer,"%d",&setIntsArray[0]);
	xil_printf("\n\r");

	xil_printf("  Enter Short Integral Stop Time in ns: -52 to 8000 <return> \t");
	ReadCommandPoll();
	sscanf(RecvBuffer,"%d",&setIntsArray[1]);
	xil_printf("\n\r");

	xil_printf("  Enter Long Integral Stop Time in ns: -52 to 8000 <return> \t");
	ReadCommandPoll();
	sscanf(RecvBuffer,"%d",&setIntsArray[2]);
	xil_printf("\n\r");

	xil_printf("  Enter Full Integral Stop Time in ns: -52 to 8000 <return> \t");
	ReadCommandPoll();
	sscanf(RecvBuffer,"%d",&setIntsArray[3]);
	xil_printf("\n\r");

	setSamples[0] = ((u32)((setIntsArray[0]+52)/4));
	setSamples[1] = ((u32)((setIntsArray[1]+52)/4));
	setSamples[2] = ((u32)((setIntsArray[2]+52)/4));
	setSamples[3] = ((u32)((setIntsArray[3]+52)/4));

	Xil_Out32 (XPAR_AXI_GPIO_0_BASEADDR, setSamples[0]);
	Xil_Out32 (XPAR_AXI_GPIO_1_BASEADDR, setSamples[1]);
	Xil_Out32 (XPAR_AXI_GPIO_2_BASEADDR, setSamples[2]);
	Xil_Out32 (XPAR_AXI_GPIO_3_BASEADDR, setSamples[3]);

	xil_printf("\n\r  Inputs Rounded to the Nearest 4 ns : Number of Samples\n\r");
	xil_printf("  Baseline Integral Window  [-200ns,%dns]: %d \n\r",-52 + ((int)Xil_In32(XPAR_AXI_GPIO_0_BASEADDR))*4, 38+Xil_In32(XPAR_AXI_GPIO_0_BASEADDR) );
	xil_printf("  Short Integral Window 	  [-200ns,%dns]: %d \n\r",-52 + ((int)Xil_In32(XPAR_AXI_GPIO_1_BASEADDR))*4, 38+Xil_In32(XPAR_AXI_GPIO_1_BASEADDR));
	xil_printf("  Long Integral Window      [-200ns,%dns]: %d \n\r",-52 + ((int)Xil_In32(XPAR_AXI_GPIO_2_BASEADDR))*4, 38+Xil_In32(XPAR_AXI_GPIO_2_BASEADDR));
	xil_printf("  Full Integral Window      [-200ns,%dns]: %d \n\r",-52 + ((int)Xil_In32(XPAR_AXI_GPIO_3_BASEADDR))*4, 38+Xil_In32(XPAR_AXI_GPIO_3_BASEADDR));

	return;
}
//////////////////////////// Set Integration Times ////////////////////////////////

//////////////////////////// PrintData ////////////////////////////////
int PrintData( ){
	u32 data;
	int dram_addr;

	//Read all data from DRAM
//	int dram_base = 0xa000000;
//	int dram_cieling = 0xa00ffff; //read out data from all integration channels

	// Read only Adj Average data from DRAM
	int dram_base = 0xa000000;
	int dram_cieling = 0xA004000; //read out just adjacent average (0xA004000 - 0xa000000 = 16384)

	Xil_DCacheInvalidateRange(0x00000000, 65536);

	for (dram_addr = dram_base; dram_addr <= dram_cieling; dram_addr+=4){
		if (!sw) { sw = XGpioPs_ReadPin(&Gpio, SW_BREAK_GPIO); } //read pin
		data = Xil_In32(dram_addr);
		xil_printf("%d\r\n",data);
		XUartPs_Recv(&Uart_PS, &RecvBuffer, 32);
		if ( RecvBuffer[0] == 'q' ) { sw = 1;  }
		if(sw) { return sw; }
	}

	return sw;
}
//////////////////////////// PrintData ////////////////////////////////


//////////////////////////// Clear Processed Data Buffers ////////////////////////////////
void ClearBuffers() {
	Xil_Out32(XPAR_AXI_GPIO_9_BASEADDR,1);
	sleep(1);						// Built in Latency ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ 1 s
	Xil_Out32(XPAR_AXI_GPIO_9_BASEADDR,0);
}
//////////////////////////// Clear Processed Data Buffers ////////////////////////////////

//////////////////////////// DAQ ////////////////////////////////
int DAQ(){
	int buffsize; 	//BRAM buffer size
	int dram_addr;	// DRAM Address
	static int filesWritten = 0;

	XUartPs_SetOptions(&Uart_PS,XUARTPS_OPTION_RESET_RX);

	Xil_Out32 (XPAR_AXI_DMA_0_BASEADDR + 0x48, 0xa000000); 		// DMA Transfer Step 1
	Xil_Out32 (XPAR_AXI_DMA_0_BASEADDR + 0x58 , 65536);			// DMA Transfer Step 2
	sleep(1);						// Built in Latency ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ 1 s
	ClearBuffers();												// Clear Buffers.
	// Capture garbage in DRAM
	for (dram_addr = 0xa000000; dram_addr <= 0xA004000; dram_addr+=4){Xil_In32(dram_addr);}

	while(1){
		buffsize = Xil_In32 (XPAR_AXI_GPIO_11_BASEADDR);
		if (!sw) { sw = XGpioPs_ReadPin(&Gpio, SW_BREAK_GPIO); } //read pin
		XUartPs_Recv(&Uart_PS, &RecvBuffer, 32);
		if ( RecvBuffer[0] == 'q' ) { sw = 1; }
		if(sw) { return sw;	}

		if(buffsize >= 4095){
			//Xil_Out32 (XPAR_AXI_GPIO_18_BASEADDR, 0); // Disable Capture Module in FPGA
			Xil_Out32 (XPAR_AXI_GPIO_15_BASEADDR, 1);
			//sleep(10);  // Built in Latency ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ 10 s
			Xil_Out32 (XPAR_AXI_DMA_0_BASEADDR + 0x48, 0xa000000);
			Xil_Out32 (XPAR_AXI_DMA_0_BASEADDR + 0x58 , 65536);
			sleep(1); 			// Built in Latency ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ 1 s
			Xil_Out32 (XPAR_AXI_GPIO_15_BASEADDR, 0);
			//Xil_Out32(XPAR_AXI_GPIO_18_BASEADDR, 1);
			ClearBuffers();
			sw = ReadDataIn(filesWritten);
			++filesWritten;
			// PrintData();
		}
	}

	return sw;
}
//////////////////////////// DAQ ////////////////////////////////

//////////////////////////// getWFDAQ ////////////////////////////////

int getWFDAQ(){
	int buffsize; 	//BRAM buffer size
	int dram_addr;	// DRAM Address

	XUartPs_SetOptions(&Uart_PS,XUARTPS_OPTION_RESET_RX);

	Xil_Out32 (XPAR_AXI_DMA_0_BASEADDR + 0x48, 0xa000000); 		// DMA Transfer Step 1
	Xil_Out32 (XPAR_AXI_DMA_0_BASEADDR + 0x58 , 65536);			// DMA Transfer Step 2
	sleep(1);						// Built in Latency ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ 1 s
	ClearBuffers();												// Clear Buffers.
	// Capture garbage in DRAM
	for (dram_addr = 0xa000000; dram_addr <= 0xA004000; dram_addr+=4){Xil_In32(dram_addr);}

	while(1){
		buffsize = Xil_In32 (XPAR_AXI_GPIO_11_BASEADDR);	// what does this look at?
		if (!sw) { sw = XGpioPs_ReadPin(&Gpio, SW_BREAK_GPIO); } //read pin
		XUartPs_Recv(&Uart_PS, &RecvBuffer, 32);
		if ( RecvBuffer[0] == 'q' ) { sw = 1; }
		if(sw) { return sw;	}

		if(buffsize >= 4095){
			//Xil_Out32 (XPAR_AXI_GPIO_18_BASEADDR, 0); // Disable Capture Module in FPGA
			Xil_Out32 (XPAR_AXI_GPIO_15_BASEADDR, 1);
			Xil_Out32 (XPAR_AXI_DMA_0_BASEADDR + 0x48, 0xa000000);
			Xil_Out32 (XPAR_AXI_DMA_0_BASEADDR + 0x58 , 65536);
			sleep(1); 			// Built in Latency ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ 1 s
			Xil_Out32 (XPAR_AXI_GPIO_15_BASEADDR, 0);
			//Xil_Out32(XPAR_AXI_GPIO_18_BASEADDR, 1);
			ClearBuffers();
			PrintData();
		}
	}
	return sw;
}

//////////////////////////// getWFDAQ ////////////////////////////////
