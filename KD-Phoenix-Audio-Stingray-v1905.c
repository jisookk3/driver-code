#name       "KD-Phoenix-Audio-Stingray"
#brand       "Phoenix"
#type         "Audio Processor"
#model      "Stingray"
#version     "1.1"
#author      "Jisook"
#comments "Phoenix Audio Stingray IP Control - Port 8888"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////PROTOCOL
//
//  IP Control over port 8888
//  Credentials are required if unit is password protected.
//  Driver is configured to login each time when login is requested
//
//  All ASCII commands must be terminated with NULL character. So for this driver, all
//  commands are converted to hex first for simplicity. This also applies for feedback.
//  The feedback is read in hex before every NULL character and then converted into string
//  for parsing.
//
//
//  Change:
//   set_ZoneSplit(int _ZoneSplit)   --->  Only this communicate via UDP
//
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////PROTOCOL

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////VARIABLE DECLARATION
extern string txtFeedback;
extern string Phoenix_IPaddress;   // for UDP communication
extern int Phoenix_portNum;   // for UDP communication

const string    ETX               = "00";                                                               // end of txt cmd
const string    WAIT            = "Please Wait...";
const string    PB_EMPTY     = "Phonebook is Empty";
const string    INVALID        = "Invalid Login. Please review credentials";
const string    ENGAGED      = "Control Engaged";
const string    ONLINE         = "Online";
const string    OFFLINE        = "Offline";
const int        ALIVE_TIME   = 6000;                                                              // 10 Minutes
const int        DEBUG_DRV   = 0;                                                                   // Flag to enable debug messages
                                                                                                                  // in driver. Set value to 0 for production
string CMD                          = "";
string StringFunctionReturnValue;

// Internals
int Device_isActive               = 0;

// Credentials if password protected
extern property string device_login;
extern property string device_password;

// Device info
int              Device_LoggedIn              = -1;
string          Device_Type                    = "";
string          Device_Name                   = "";
int              Device_Status_Usb            = -1;
int              Device_Status_Phone        = -1;
int              Device_Status_DND          = -1;
int              Device_Status_PhoneHD    = -1;
int              Device_Status_Split           = -1;

// Phone book variables
string          Device_Selected_PhonebookNumber       = "";
string          Device_Selected_DialerNumber               = "";
int              Device_Selected_PhoneList                     = 0;

// Phone book meta variables
int              #Index_Device_Phonebook_Metadata;
metadata     Device_Phonebook_Metadata[#Index_Device_Phonebook_Metadata];
string          Device_Phonebook_Metadata.title;                                               // Name
string          Device_Phonebook_Metadata.artist;                                             // Number
string          Device_Phonebook_Metadata.album;                                           // Call type
string          Device_Phonebook_Metadata.category;                                       // Time of call
string          Device_Phonebook_Metadata.genres;                                          // Call duration

// Dialer
int              Device_HookState           = -1;
string          Device_CallStatus           = "";

// Mute variables
int              Device_Mute_Podium;                                                                // 21
int              Device_Mute_Microphone;                                                           // 22
int              Device_Mute_SystemOutput;                                                       // 23
int              Device_Mute_Speakers;                                                              // 24

// Volume Variables
int              Device_Volume_Podium;                                                           // 21
int              Device_Volume_Microphone;                                                     // 22
int              Device_Volume_SystemOutput;                                                 // 23
int              Device_Volume_Speakers;                                                        // 24

// Line Type Variables
int             #Index_Device_LineType;
string         Device_LineType[#Index_Device_LineType];

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////FUNCTION DECLARATION
extern internal_Update(string);
extern internal_Parse(string);
extern internal_SendCmd(string);

extern credentials_SendLogin();
extern credentials_SendLogout();
extern Keep_Alive();

extern internal_CheckLogin(string);
extern internal_ButtonsNames();

extern get_ContactList();
extern get_RecentCallList();
extern get_DeviceStatus();
extern get_CallStatus();
extern get_Mute(int);
extern get_Volume(int);
extern get_LineType();
extern get_Dial_CallStatus();
extern GetStatus();

extern parse_Login(string);
extern parse_HookState(string);
extern parse_DeviceStatus(string);
extern parse_CallStatus(string);
extern parse_Volume(string);
extern parse_Mute(string);
extern parse_LineType(string);
extern parse_Contacts(string);
extern parse_Calls(string);

extern clear_Phonebook();
extern clear_Phonebook_Empty();
extern clear_Phonebook_Wait();

// Metadata Phonebook
extern setDatabaseSelect(int);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////PARAMETER ARRAYS

#param_array _MuteChannels "Podium Lines" = "21", "Microphones" = "22", "System Output" = "23", "Speakers" = "24";

#param_array _VolumeChannels "Speakers" = "24";

#param_array _Mute "Off" = "0", "On" = "1";

#param_array _ZoneSplit "Room Combined" = "0", "Room Split" = "1";

#param_array _KeyCode "Dial 0" = "0", "Dial 1" = "1", "Dial 2" = "2", "Dial 3" = "3", "Dial 4" = "4", "Dial 5" = "5", "Dial 6" = "6",
                                    "Dial 7" = "7", "Dial 8" = "8", "Dial 9" = "9", "Dial *" = "10", "Dial #" = "11", "Speaker Volume Up" = "17",
                                    "Speaker Vol Down" = "18", "Call Send" = "19", "Call End" = "20" ;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////COMPASS FUNCTIONS
main()
{
     string Response, data;

     // Kill if not active
     if(Device_isActive != 1){ return; }
     
     // Get Response
     OpenControl(0);
     GetHexBefore(Response, ETX);

     // Convert to string
     HexToString(data, Response);

     // DEBUG - responses from device
     if (DEBUG_DRV == 1){ txtFeedback = data + "\r\n"; }

     // Parse
     internal_Parse(data);
}


init()
{
}


wakeup()
{
}


Start()
{
     // reset phone book
     clear_Phonebook_Empty();

     Device_isActive = 1;
     OpenControl(0);
     Delay(0.5);
     credentials_SendLogin();
}


Stop()
{
     Device_isActive = 0;
     Device_LoggedIn = -1;
     Device_HookState = -1;
     Device_Type = "";
     Device_Name = "";
     CloseControl();

     // reset phone book
     clear_Phonebook_Empty();
}


GetStatus()
{
     // Get volume (21-24)
     get_Mute(21);
     Delay(0.1);
     get_Mute(22);
     Delay(0.1);
     get_Mute(23);
     Delay(0.1);
     get_Mute(24);
     Delay(0.1);

     // Get volume (24)
     get_Volume(24);
     Delay(0.1);

     // Get line types
     get_LineType();
     Delay(0.1);

     //Get device status
     get_DeviceStatus();
     Delay(0.1);

     // Get call status
     get_Dial_CallStatus();
     Delay(0.1);
}


clear_Phonebook()
{
     // Clear Phonebook Database
     free(Device_Phonebook_Metadata);
     Device_Selected_PhonebookNumber = "";
     #Index_Device_Phonebook_Metadata = 1;
     Device_Phonebook_Metadata.title = "";                                               // name
     Device_Phonebook_Metadata.artist = "";                                             // number
     Device_Phonebook_Metadata.album = "";                                           // Call type
     Device_Phonebook_Metadata.category = "";                                       // Time of call
     Device_Phonebook_Metadata.genres = "";                                          // Call duration
     #Index_Device_Phonebook_Metadata = 0;
}


clear_Phonebook_Wait()
{
     LockStart();
         clear_Phonebook();
         #Index_Device_Phonebook_Metadata = 1;
         Device_Phonebook_Metadata.title = WAIT;                                          // name
         #Index_Device_Phonebook_Metadata = 0;
     LockStop();
}


clear_Phonebook_Empty()
{
     LockStart();
         clear_Phonebook();
         //#Index_Device_Phonebook_Metadata = 1;
         //Device_Phonebook_Metadata.title = PB_EMPTY;
         //#Index_Device_Phonebook_Metadata = 0;
     LockStop();
}


setDatabaseSelect(int numSelect)
{
     string number;

     // Pull Info From Selected Phonebook
     LockStart();
        #Index_Device_Phonebook_Metadata = numSelect;
        number = Device_Phonebook_Metadata.artist;
     LockStop();

     Device_Selected_PhonebookNumber = number;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////LOGIN COMMANDS
credentials_SendLogin()
{
     // Cmd
     sprintf(CMD, "LOGIN1: %s %s", device_login, device_password);

     // Send
     internal_SendCmd(CMD);
}


credentials_SendLogout()
{
     // Cmd
     sprintf(CMD, "STOP:");

     // Send
     internal_SendCmd(CMD);
}


Keep_Alive()
{
     credentials_SendLogin();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////SET COMMANDS
set_Mute(int _MuteChannels, int _Mute)
{
     // Cmd
     sprintf(CMD, "SET_MUTE: %d %d", _MuteChannels, _Mute);

     // Send
     internal_SendCmd(CMD);
}


set_Volume(int _VolumeChannels, int value_0_to_100)
{
     // Cmd
     sprintf(CMD, "SET_VOLUME: %d %d", _VolumeChannels, value_0_to_100);

     // Send
     internal_SendCmd(CMD);
}


set_KeyPad(int _KeyCode)
{
     // Cmd
     sprintf(CMD, "KEY: %d", _KeyCode);

     // Send
     internal_SendCmd(CMD);
}


set_DialNumber(string PhoneNumber)
{
     // Cmd
     sprintf(CMD, "DIAL: %s", PhoneNumber);

     // Send
     internal_SendCmd(CMD);
}


set_ZoneSplit(int _ZoneSplit)
{
     string hexCmd;     // Cmd

     sprintf(CMD, "SPLIT_ZONE: %d", _ZoneSplit);

     // Convert to hex
     StringToHex(hexCmd, CMD);

     // Append terminating character to command
     sprintf(hexCmd, "%s,%s", hexCmd, ETX);

     // Send command via UDP for ONLY Zone Split command
     OpenControl(0);
     SendUDP(Phoenix_IPaddress, Phoenix_portNum, hexCmd);     // Send
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////GET COMMANDS
get_Mute(int _MuteChannels)
{
     // Cmd
     sprintf(CMD, "GET_MUTE: %d", _MuteChannels);

     // Send
     internal_SendCmd(CMD);
}


get_Volume(int _VolumeChannels)
{
     // Cmd
     sprintf(CMD, "GET_VOLUME: %d", _VolumeChannels);

     // Send
     internal_SendCmd(CMD);
}


get_DeviceStatus()
{
     // Cmd
     sprintf(CMD, "KEY: 30");

     // Send
     internal_SendCmd(CMD);
}


get_LineType()
{
     // Cmd
     sprintf(CMD, "GET_LINE_TYPE:");

     // Send
     internal_SendCmd(CMD);
}


get_Dial_ContactList()
{
     // Cmd
     sprintf(CMD, "KEY: 23");

     // Send
     internal_SendCmd(CMD);
}


get_Dial_AllRecentCalls()
{
     // Cmd
     sprintf(CMD, "KEY: 24");

     // Send
     internal_SendCmd(CMD);
}


get_Dial_CallStatus()
{
     // Cmd
     sprintf(CMD, "KEY: 31");

     // Send
     internal_SendCmd(CMD);
}


get_Dial_DialedCalls()
{
     // Cmd
     sprintf(CMD, "GET_DIALED:");

     // Send
     internal_SendCmd(CMD);
}


get_Dial_ReceivedCalls()
{
     // Cmd
     sprintf(CMD, "GET_RECEIVED:");

     // Send
     internal_SendCmd(CMD);
}


get_Dial_MissedCalls()
{
     // Cmd
     sprintf(CMD, "GET_MISSED:");

     // Send
     internal_SendCmd(CMD);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////PARSING FUNCTIONS
internal_Parse(string Response)
{
     string cmd, data, char, fData;

     // Parse response - set cmd to upper case
     parse(Response, "%s:%s", cmd, data);
     SetUpperCase(cmd, cmd);

     // strip any white spaces in the preceding data
     do
     {
          StringCut(data, 1, char, data);
     }while(char == " ");

     // Reconstruct data
     sprintf(data, "%s%s", char, data);

     // Send data to parse function
     switch(cmd){
          case "LOGIN":
               parse_Login(data);
               break;
          case "HOOK_STATE":
               parse_HookState(data);
               break;
          case "DEVICE_STATUS":
               parse_DeviceStatus(data);
               break;
          case "CALL_STATUS":
               parse_CallStatus(data);
               break;
          case "VOLUME_LEVEL":
               parse_Volume(data);
               break;
          case "MUTE_STATUS":
               parse_Mute(data);
               break;
          case "LINE_TYPE":
               parse_LineType(data);
               break;
          case "CONTACTS":
               LockStart();
                    parse_Contacts(data);
               LockStop();
               break;
          case "CALLS":
               LockStart();
                    parse_Calls(data);
               LockStop();
               break;
          default:
               // DEBUG
               if (DEBUG_DRV == 1){ sprintf(txtFeedback, "WARNING: Ignoring feedback. Unknown response from '%s'\r\n", cmd); }
               break;
     }
}


parse_Login(string data)
{
     parse(data, "%d %s %s", Device_LoggedIn, Device_Type, Device_Name);
}


parse_HookState(string data)
{
     string state, upper;

     parse(data, "%s", state);
     SetUpperCase(upper, state);
     if (upper == "ON")
     {
          Device_HookState = 1;
     }
     else
     {
          Device_HookState = 0;
     }
}


parse_DeviceStatus(string data)
{
     int code;

     parse(data, "%d", code);
     switch (code)
     {
          case 10:
               Device_Status_Usb = 0;
               break;
          case 11:
               Device_Status_Usb = 1;
               break;
          case 20:
               Device_Status_Phone = 0;
               break;
          case 21:
               Device_Status_Phone = 1;
               break;
          case 30:
               Device_Status_DND = 0;
               break;
          case 31:
               Device_Status_DND = 1;
               break;
          case 40:
               Device_Status_PhoneHD = 0;
               break;
          case 41:
               Device_Status_PhoneHD = 1;
               break;
          default:
               // DEBUG
               if (DEBUG_DRV == 1){ sprintf(txtFeedback, "WARNING: Status code not supported (DEVICE_STATUS: %d)\r\n", code); }
               break;
     }
}


parse_CallStatus(string data)
{
     int          code;
     string      msg;
     string      number;

     parse(data, "%d", code);
     parse(data, "%d %s", code, number);

     switch (code)
     {
          case 0:
               msg = "Idle";
               break;
          case 5:
               msg = "Ringing";
               break;
          case 6:
               msg = "Dialing";
               break;
          case 7:
               msg = "Incoming Call";
               break;
          case 8:
               msg = "Outgoing Call";
               break;
          case 9:
               msg = "Waiting Call";
               break;
          case 15:
               msg = "Conference Call";
               break;
          default:
               msg = "";
               // DEBUG
               if (DEBUG_DRV == 1){ sprintf(txtFeedback, "WARNING: Status code not supported (CALL_STATUS: %d)\r\n", code); }
               break;
     }
     Device_CallStatus = msg;
     Device_Selected_DialerNumber = number;
}


parse_Volume(string data)
{
     int code;
     int value;

     parse(data, "%d %d", code, value);
     switch (code)
     {
          case 21:
               Device_Volume_Podium = value;
               break;
          case 22:
               Device_Volume_Microphone = value;
               break;
          case 23:
               Device_Volume_SystemOutput = value;
               break;
          case 24:
               Device_Volume_Speakers = value;
               break;
          default:
               // DEBUG
               if (DEBUG_DRV == 1){ sprintf(txtFeedback, "WARNING: Channel code not supported (VOLUME_LEVEL: %d)\r\n", code); }
               break;
     }
}


parse_Mute(string data)
{
     int code;
     int value;

     parse(data, "%d %d", code, value);
     switch (code)
     {
          case 21:
               Device_Mute_Podium = value;
               break;
          case 22:
               Device_Mute_Microphone = value;
               break;
          case 23:
               Device_Mute_SystemOutput = value;
               break;
          case 24:
               Device_Mute_Speakers = value;
               break;
          default:
               // DEBUG
               if (DEBUG_DRV == 1){ sprintf(txtFeedback, "WARNING: Channel code not supported (MUTE_STATUS: %d)\r\n", code); }
               break;
     }
}


parse_LineType(string data)
{
     int x, line1, line2, line3, line4;
     int value;
     string type;

     parse(data, "%d %d %d %d", line1, line2, line3, line4);

     // Loop through each one. Funny business going on here...
     for(x = 1; x <= 4; x = x + 1)
     {
          value = 0;
          type = "";

          // Get type integer
          if (x == 1)
          {
               value = line1;
          }
          else if (x == 2)
          {
               value = line2;
          }
          else if (x == 3)
          {
               value = line3;
          }
          else if (x == 4)
          {
               value = line4;
          }

          // Resolve type string
          switch (value)
          {
               case 0:
                    type = "Mixer";
                    break;
               case 1:
                    type = "Podium";
                    break;
               case 2:
                    type = "Auxiliary";
                    break;
               case 15:
                    type = "Not Set";
                    break;
               default:
                    // DEBUG
                    if (DEBUG_DRV == 1){ sprintf(txtFeedback, "WARNING: Line Type not supported (#%d LINE_TYPE: %d)\r\n", x, value); }
                    break;
          }

          // Save the value into the correct variable
          #Index_Device_LineType = x;
          Device_LineType = type;
     }
}


parse_Contacts(string data)
{
     string char, contact;
     int x, Length, tLen;

     // Do not populate phonebook if calls is selected
     if (Device_Selected_PhoneList != 0){ return; }

     char = ""; contact = "";
     strlen(data, Length);
     tLen = Length;

     // Loop through contacts adding them one by one
     for(x = 0; x < tLen; x = x + 1)
     {
          StringCut(data, 1, char, data);
          if((char == ";") || (x + 1 == tLen))
          {
               if ((x + 1 == tLen) && (char != ";")) { sprintf(contact, contact + char); }
               parse(contact, "%d %s %s", #Index_Device_Phonebook_Metadata, Device_Phonebook_Metadata.artist, Device_Phonebook_Metadata.title);
               contact = "";
          }
          else{ sprintf(contact, contact + char); }
     }
}


parse_Calls(string data)
{
     string char, char2, contact, name, time;
     string duration, tmp, tmp2, tmp3, msg, msg2, msg3;
     string ws_char;
     int x, xx, Length, tLen, Length2, tLen2, type;

     // Do not populate calls if phone book is selected
     if (Device_Selected_PhoneList == 0){ return; }

     char = ""; contact = "";
     strlen(data, Length);
     tLen = Length;

     // Loop through calls adding them one by one
     for(x = 0; x < tLen; x = x + 1)
     {
          StringCut(data, 1, char, data);
          if((char == ";") || (x + 1 == tLen))
          {
               // Init
               if ((x + 1 == tLen) && (char != ";")) { sprintf(contact, contact + char); }

               // strip any white spaces in the preceding data
               do
               {
                    StringCut(contact, 1, ws_char, contact);
               }while(ws_char == " ");

               // Reconstruct data
               sprintf(contact, "%s%s", ws_char, contact);

               parse(contact, "%d %d %s", #Index_Device_Phonebook_Metadata, type, tmp);
               parse(tmp, "%s %s %s", Device_Phonebook_Metadata.artist, Device_Phonebook_Metadata.title, tmp2);
               parse(tmp2, "%s %s", time, duration);

               // Resolve call type
               if (type == 0)
               {
                    tmp3 = "Dialed Call";
               }
               else if (type == 1)
               {
                    tmp3 = "Received Call";
               }
               else if (type == 2)
               {
                    tmp3 = "Missed Call";
               }
               else
               {
                    // DEBUG
                    if (DEBUG_DRV == 1){ sprintf(txtFeedback, "WARNING: Call Type not supported (CALLS: <type> %d)\r\n", type); }
               }

               // Put msg together for single line of data
               sprintf(msg, "%s - %s", tmp3, time);
               sprintf(msg2, "%s - %s", msg, duration);

               // Replace '^' carrot with spaces. Protocol said so...
               strlen(msg2, Length2);
               tLen2 = Length2;
               for(xx = 0; xx < tLen2; xx = xx + 1)
               {
                    StringCut(msg2, 1, char2, msg2);
                    if (char2 == "^"){ sprintf(msg3, msg3 + " "); }
                    else{ sprintf(msg3, msg3 + char2); }
               }
               parse(msg3, "%s", Device_Phonebook_Metadata.album);

               // Clear stuff
               char = "";
               char2 = "";
               ws_char = "";
               contact = "";
               msg = "";
               msg2 = "";
               msg3 = "";
               tmp = "";
               tmp2 = "";
               tmp3 = "";
               time = "";
               duration = "";
          }
          else{ sprintf(contact, contact + char); }
     }
}



internal_SendCmd(string cmd)
{
     string hexCmd;

     // Convert to hex
     StringToHex(hexCmd, cmd);

     // Append terminating character to command
     sprintf(hexCmd, "%s,%s", hexCmd, ETX);

     // Send
     OpenControl(0);
     SendHex(hexCmd);
}

