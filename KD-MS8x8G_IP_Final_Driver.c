#name    "KD-KeyDigital-Switch-MS8x8G-v1902"
#brand   "Key Digital"
#type    "HDMI Switcher"
#model   "MS8x8G"
#version "1.0"
#author  "JISOOK"
#comments "Real Time Module Improved"


//  constant variables for MS8x8G switcher, In/Output index
const int MAX_INPUTS = 8;
const int MAX_OUTPUTS = 8;
int #Index_KD_Outputs;
int #Index_KD_Inputs;

//  Array declaration for Input/Output names & 8x8 Matrix switch configuration
string KD_Output_Name[8#Index_KD_Outputs];
string KD_Input_Name[8#Index_KD_Inputs];
int KD_Output_Selected_Input[8#Index_KD_Outputs];
int KD_Output_Selected_Input_Audio[8#Index_KD_Outputs];

int KD_Active;
int KD_State;

//  Selected IN/OUTPUT variables from a Module
int KD_Selected_Output;
int KD_Selected_Input;

//  For writing IN/OUTPUT names from a Module
int KD_Temp_Index;
string KD_Temp_Name;

//  For storing all data from iOS buffer
string String_Buffer;
string Debug;

//  0 = Audio Video switch, 1 = Video Only switch, 2 = Audio Only switch
//  This is linked to a button in Module
int KD_AV_Mode;
int KD_STA_Bypass;

//  0 = Single switch, 1 = All output switch, 2 = Pass through switch
int KD_Output_Mode;

//  For reading status of the device in real time
int KD_Real_Time_Delay;

//  Pass-through switch check FLAG from a Module
int KD_Pass_Through_Ready;

//  All functions declaration
extern Start_Control();
extern Stop_Control();
extern Send_Command(string);
extern Set_Output_Name(int);
extern Set_Internal_Memory(int);
extern Set_Default_Names();
extern Get_Status();
extern Get_Input_Name();
extern Get_Output_Name();
extern Set_AV_Switch();
extern Set_AV_Switch_All();
extern Get_Status_All();
extern Set_AV_Switch_Passthrough();
extern Run_Switch();
extern Get_Current_Input();

//  Initialize all variables when a module runs at FIRST.
init() {
     LockStart();
          Set_Default_Names();
     LockStop();
     KD_State = 0;
     String_Buffer = "";
     KD_STA_Bypass = 0;
     KD_Selected_Output = 1;
     KD_Selected_Input = 1;
     KD_Output_Mode = 0;
     KD_AV_Mode =0;
     KD_Real_Time_Delay = 0;
     KD_Pass_Through_Ready = 1;
    // Debug = "";
}

//  When a module stops and runs again
wakeup() {
     Stop_Control();
     Start_Control();
}

//  Main process = Read Status -> Update current in/output -> get in/output names -> count 10s (Real-time driver) -> go back to Read Status.
main() {
     if(KD_Active == 0) return;

     switch(KD_State){

          //  AT the beginning of the module, KD_State = 0. Update status of the device.
          case 0:
               Get_Status();
               KD_State = 1;
               break;

          //  Update current input/outputs and show them in GUI
          case 1:
               Get_Current_Input();
               KD_State = 2;
               break;

          //  Update Input Names
          //  KD_STA_Bypass == 1, if input name is changed by a user. 
          //  Then, skip to update output name (SKIP case 3:) and jump to a timer for a real-time driver (GOTO case 4:)
          case 2:
               Get_Input_Name();
               if(KD_STA_Bypass == 1) {
                    KD_State = 4;
                    KD_STA_Bypass = 0;
               } else KD_State = 3;      
               break;

          //  Update Output Names
          //  KD_STA_Bypass == 1, if output name is changed by a user. 
          //  Then, jump to a timer for a real-time driver (GOTO case 4:)
          //  If not, set the timer 10 seconds to read a status (KD_Real_Time_Delay = 10;) and then GOTO case 4:
          case 3:
               Get_Output_Name();
               if(KD_STA_Bypass == 1) {
                    KD_State = 4;
                    KD_STA_Bypass = 0;
               } else {
                    KD_State = 4;
                    KD_Real_Time_Delay = 10;
               }
               break;

          //  For a real-time driver, need to count 10s (in this case) and then call the beginning of the main to update status
          //  Thus, call Get_Status() every 10s
          case 4:
               if(KD_Real_Time_Delay == 0)
                    KD_State = 0;
               else 
                    KD_Real_Time_Delay = KD_Real_Time_Delay - 1;
          break;

          default:
          break;
     }
}

Start_Control() {
     KD_Active = 1;
     KD_State = 0;
}

Stop_Control() {
     KD_Active = 0;
     CloseControl();
}

//  For a half-way driver. Run the function to refresh the STATUS whenever KD_AV_Mode button pressed.
//  Not using for a real-time driver.
Get_Status_All() {
     KD_STA_Bypass = 0;
     KD_State = 0;
}

//  For sending a command to open/close connection
Send_Command(string command) {
     string cmd;
     cmd = command + "\r";
     //sprintf(Debug,Debug + "Command: %s", cmd);
     OpenControl(0);
//     GetControlBefore(clear);
     SendControl(cmd);
     Delay(0.2);
     CloseControl();
}

//  If a user changes input name from a module, call Get_Input_Name() ===> KD_State = 2 (look at the main func.)
Set_Input_Name(int input, string name) {
     string cmd;
     sprintf(cmd,"SPI%02dWN%s",input,name);
     Send_Command(cmd);
     //  Call Get_Input_Name()
     KD_State = 2;
     KD_STA_Bypass = 1;
}

//  If a user changes input name from a module, call Get_Output_Name() ===> KD_State = 3 (look at the main func.)
Set_Output_Name(int output, string name) {
     string cmd;
     sprintf(cmd,"SPO%02dWN%s",output,name);
     Send_Command(cmd);
     //  Call Get_Output_Name()
     KD_State = 3;
     KD_STA_Bypass = 1;
}

//  For storing a matrix switch configuration for all outputs switched to one input
Set_Internal_Memory(int Input) {
     SetArray(KD_Output_Selected_Input, Input);
}

//  Set default names of In/Outputs
Set_Default_Names() {
     int i;
     for(i = 1; i <= MAX_INPUTS; i = i + 1) {
          #Index_KD_Inputs = i;
          #Index_KD_Outputs = i;
          sprintf(KD_Input_Name,"Input %d", i);
          sprintf(KD_Output_Name,"Output %d", i);
     }
}

//  Send a "STA" command to read STATUS of the device -> get return messages from iOS buffer -> parse all data
Get_Status() {
     string Response, clear, temp_buffer, output, input, char, packet;
     int parse_state, buffer_length, i, temp;
     Response = "null"; clear  = ""; temp_buffer =""; char = ""; packet = "";
     parse_state = 0; buffer_length = 0;

     //  Due to large data of status, need a Delay(0.3) to read all from buffer and store them to "String_Buffer".
     OpenControl(0);
     GetControlBefore(clear);
     SendControl("STA\r");
     String_Buffer = "";
     while(Response != "") {
          Response = "";
          Delay(0.3);
          GetControlBefore(Response);
          LockStart();
          String_Buffer = String_Buffer + Response;
          LockStop();
     }
     CloseControl();

     //  save buffer data to temp and get the length of buffer.
     temp_buffer = String_Buffer;
     strlen(temp_buffer, buffer_length);

     //  Cut one character and check if it is "--". If found, read data until "\r" (one line reading).
     //  Then parsing the data + update matrix switch configuration depending on AV switch or Audio only switch mode. 
     for(i = 1; i <= buffer_length; i = i +1) {
          StringCut(temp_buffer, 1, char, temp_buffer);
          switch(parse_state) {
               //  find the first "-" for the start of STATUS
               case 0:
                    if(char == "-")
                         parse_state = 1;
                    break;

               //  find the second "-"
               case 1:
                    if(char == "-") {
                         parse_state = 2;
                         packet ="";  // packet clear. No need to parse of "--"
                    } else
                         parse_state = 0;
                    break;

               //  Parse part: extract one line (until "/r") and store it to packet -> parse data
               case 2:
                    if(char == "\r") {
                         parse_state = 0;
                         //  Parse for Audio and Video switch status
                         //  Example STATUS message: -- HDMI Output 01 : IN = 01
                         output = ""; input = "";
                         parse(packet, "HDMI Output %02d : IN = %02d", output, input);
                         //  Exception Process! Check for empty data and proper range of In/Output
                         if(output != "" && input != "") {
                              LockStart();
                              sprintf(temp, "%d", output);
                              if( temp > 0 && temp <= MAX_OUTPUTS ){
                                   #Index_KD_Outputs = temp;
                                   sprintf(temp, "%d", input);
                                   if( temp > 0 && temp <= MAX_INPUTS ){
                                        KD_Output_Selected_Input = temp;
                                   }
                              }
                              LockStop();
                         }
                         //  Parse for Audio Only switch status
                         output = ""; input = "";     
                         parse(packet, "Audio Output %02d : IN = %02d", output, input);
                         //  Exception Process! Check for empty data and out of range of In/Output
                         if(output != "" && input != "") {
                              LockStart();
                              sprintf(temp, "%d", output);
                              if( temp > 0 && temp <= MAX_OUTPUTS ){
                                   #Index_KD_Outputs = temp;
                                   sprintf(temp, "%d", input);
                                   if( temp > 0 && temp <= MAX_INPUTS ){
                                        KD_Output_Selected_Input_Audio = temp;
                                   }
                              }
                              LockStop();
                         }
                         packet = "";
                    } // case 2
                    else {
                         //  accumulate each character to packet until found "\r"
                         packet = packet + char;    
                    }
                    break;
          }  //switch
     } // for
}

//  Send a command to read all inputs' names -> parse data -> update names
Get_Input_Name() {
     string Response, clear, cmd, temp_buffer, cmd_result, message, name, index, char, packet;
     int parse_state, buffer_length, i, temp;
     Response = "null"; clear = ""; cmd = ""; temp_buffer =""; cmd_result = ""; message = ""; name = ""; index = "";
     char = ""; packet = "";
     parse_state = 0; buffer_length = 0;

     //  Make a command to read all inputs' names
     for(i = 1; i <= MAX_INPUTS; i = i + 1)
          sprintf(cmd, cmd + "\rSPI %02d RN\r", i);

     OpenControl(0);
     GetControlBefore(clear);
     SendControl(cmd);
     String_Buffer = "";
     while(Response != "") {
          Response = "";
          Delay(0.2);
          GetControlBefore(Response);
          LockStart();
          String_Buffer = String_Buffer + Response;
          LockStop();
     }
     CloseControl();

     temp_buffer = String_Buffer;
     strlen(temp_buffer, buffer_length);

     //  Cut one character and check if it is "<". If found, read data until "\r" (one line reading).
     //  Then parsing the data + update input names
     for(i = 1; i <= buffer_length; i = i +1) {
          StringCut(temp_buffer, 1, char, temp_buffer);
          switch(parse_state) {
               //  Find the first character "<" for a start of Input name data
               case 0:
                    if(char == "<") {
                         parse_state = 1;
                         packet = packet + char;
                    }
                    break;

               //  Parse part: extract one line (until "/r") and store it to packet -> parse data
               //  Example a return message: <s>SPI01RN</s><user>Input 01's Name: Input 1</user>
               case 1:
                    if(char == "\r") {
                         parse_state = 0; 
                         cmd_result = ""; message = ""; name = ""; index = "";
                         parse(packet, "<s>%s</s><user>%s: %s</user>", cmd_result, message, name);
                         parse(cmd_result, "SPI%02dRN", index);

                         //  Exception Process! Check for empty data and out of range of In/Output index
                         sprintf(temp, "%d", index);
                         if(temp > 0 && temp <= MAX_INPUTS) {
                              //  If not found name, set default name
                              if(name == "") {
                                   sprintf(name, "Input %02d", temp);
                              }
                              LockStart();
                              #Index_KD_Inputs = temp;
                              KD_Input_Name = name;
                              LockStop();
                         }
                         packet = "";
                    } else
                         packet = packet + char;
                    break;
          } // swtich
     } // for
}

//  Send a command to read all outputs' names -> parse data -> update names
Get_Output_Name() {
     string Response, clear, cmd, temp_buffer, cmd_result, message, name, index, char, packet;
     int parse_state, buffer_length, i, temp;
     Response = "null"; clear = ""; cmd = ""; temp_buffer =""; cmd_result = ""; message = ""; name = ""; index = "";
     char = ""; packet = "";
     parse_state = 0; buffer_length = 0;

     //  Make a command to read all outputs' names
     for(i = 1; i <= MAX_OUTPUTS; i = i + 1)
          sprintf(cmd, cmd + "\rSPO %02d RN\r", i);

     OpenControl(0);
     GetControlBefore(clear);
     SendControl(cmd);
     String_Buffer = "";
     while(Response != "") {
          Response = "";
          Delay(0.2);
          GetControlBefore(Response);
          LockStart();
          String_Buffer = String_Buffer + Response;
          LockStop();
     }
     CloseControl();

     temp_buffer = String_Buffer;
     strlen(temp_buffer, buffer_length);

     //  Cut one character and check if it is "<". If found, read data until "\r" (one line reading).
     //  Then parsing the data + update output names
     for(i = 1; i <= buffer_length; i = i +1) {
          StringCut(temp_buffer, 1, char, temp_buffer);
          switch(parse_state) {
               case 0:
                    if(char == "<") {
                         parse_state = 1;
                         packet = packet + char;
                    }
               break;
  
               case 1:
                    if(char == "\r") {
                         parse_state = 0;
                         parse(packet, "<s>%s</s><user>%s: %s</user>", cmd_result, message, name);
                         parse(cmd_result, "SPO%02dRN", index);

                         //  Exception Process! Check for empty data and out of range of In/Output index
                         sprintf(temp, "%d", index);
                         if(temp > 0 && temp <= MAX_OUTPUTS) {
                              if(name == "") {
                                   sprintf(name, "Output %02d", temp);
                              }
                              LockStart();
                              #Index_KD_Outputs = temp;
                              KD_Output_Name = name;
                              LockStop();
                         }
                         packet = "";
                    } else {
                         packet = packet + char;
                    }
               break;
          }
     }
}

//  3 SINGLE switch(KD_AV_Mode) - AV(0), Video only(1), Audio only(2) ==> ONE OUTPUT switched to ONE INPUT
//  Send the command depending on the switch mode that a user selects from a module -> update the matrix switch configuration
Set_AV_Switch() {
     string cmd, clear;
     cmd = ""; clear = "";

     #Index_KD_Outputs = KD_Selected_Output;

     //  (KD_AV_Mode) 0 = Audio Video switch, 1 = Video Only switch, 2 = Audio Only switch
     switch(KD_AV_Mode) {
          case 0: // AV switch mode
               sprintf(cmd, "SPO%02dSI%02d", #Index_KD_Outputs, KD_Selected_Input);
               KD_Output_Selected_Input = KD_Selected_Input;
               KD_Output_Selected_Input_Audio = KD_Selected_Input;
               break;

          case 1: // Video Only switch
               sprintf(cmd, "SPO%02dSB%02d", #Index_KD_Outputs, KD_Selected_Input);
               KD_Output_Selected_Input = KD_Selected_Input;
               break;

          case 2: // Audio Only switch
               sprintf(cmd, "SPO%02dSA%02d", #Index_KD_Outputs, KD_Selected_Input);
               KD_Output_Selected_Input_Audio = KD_Selected_Input;
               break;

          default:
               KD_AV_Mode = 0;
               break;
     }

     GetControlBefore(clear);
     Send_Command(cmd);
}

//  3 ALL Output switch(KD_AV_Mode) - AV(0), Video only(1), Audio only(2) ==> ALL OUTPUTs switched to ONE INPUT
//  Send the command depending on the switch mode that a user selects from the module -> update matrix switch configuration
Set_AV_Switch_All() {
     string cmd, clear;
     cmd = ""; clear = "";

     //  (KD_AV_Mode) 0 = Audio Video switch, 1 = Video Only switch, 2 = Audio Only switch
     switch(KD_AV_Mode) {
          case 0: // AV switch mode
               sprintf(cmd, "SPOASI%02d", KD_Selected_Input);
               //  SetArray() --> For storing data of selected one input for ALL OUTPUTS
               SetArray(KD_Output_Selected_Input, KD_Selected_Input);
               SetArray(KD_Output_Selected_Input_Audio, KD_Selected_Input);           
               break;

          case 1: // Video Only switch
               sprintf(cmd, "SPOASB%02d", KD_Selected_Input);
               SetArray(KD_Output_Selected_Input, KD_Selected_Input);
               break;

          case 2: // Audio Only switch
               sprintf(cmd, "SPOASA%02d", KD_Selected_Input);
               SetArray(KD_Output_Selected_Input_Audio, KD_Selected_Input);
               break;

          default:
               KD_AV_Mode = 0;
               break;
     }

     GetControlBefore(clear);
     Send_Command(cmd);
}

//  Pass-through switch: input 1 output 1, input 2 output 2, and so on...
//  Send the pass-through command -> update a matrix switch configuration
Set_AV_Switch_Passthrough() {
     string cmd, clear;
     int i;
     cmd = ""; clear = "";

     //  Make a pass-through In/Outputs
     for(i = 1; i <= MAX_INPUTS ; i = i + 1) {
          #Index_KD_Outputs = i;
          KD_Selected_Input = i;
          KD_Selected_Output = i;

          //  (KD_AV_Mode) 0 = Audio Video switch, 1 = Video Only switch, 2 = Audio Only switch
          switch(KD_AV_Mode) {
               case 0: // AV switch mode
                    sprintf(cmd, "SPO%02dSI%02d", i, i);
                    KD_Output_Selected_Input = i;
                    KD_Output_Selected_Input_Audio = i;
                    break;

               case 1: // Video Only switch
                    sprintf(cmd, "SPO%02dSB%02d", i, i);
                    KD_Output_Selected_Input = i;
                    break;

               case 2: // Audio Only switch
                    sprintf(cmd, "SPO%02dSA%02d", i, i);
                    KD_Output_Selected_Input_Audio = i;
                    break;

               default:
                    KD_AV_Mode = 0;
                    break;
          }
          GetControlBefore(clear);
          Send_Command(cmd);
     }
     //  pass-through FLAG for a module
     KD_Pass_Through_Ready = 1;
}

//  KD_Output_Mode - Single switch(0), All outputs switch(1), Pass-through switch(2)
//  Call the proper switch mode function
Run_Switch() {
     switch(KD_Output_Mode) {
          case 0: // Single switch
               Set_AV_Switch();
               break;

          case 1: // All outputs switch
               Set_AV_Switch_All();
               break;

          case 2: // Pass-through switch
               Set_AV_Switch_Passthrough();
               break;

          default:
               Set_AV_Switch();
               break;
     }
}

//  To fix the compass control issue to run between an compass control event(module) and a driver
//  Use for the initial current input reading in the main function
Get_Current_Input() {
     #Index_KD_Outputs = KD_Selected_Output;

     switch(KD_AV_Mode) {
               case 0: // AV Switch
               KD_Selected_Input = KD_Output_Selected_Input;
               break;

          case 1: // Video Only Switch
               KD_Selected_Input = KD_Output_Selected_Input;
               break;

          case 2: // Audio Only Switch
               KD_Selected_Input = KD_Output_Selected_Input_Audio;
               break;

          default:
               KD_AV_Mode = 0;
               break;
     }
}