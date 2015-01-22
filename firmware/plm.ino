
const char SEND_MSG_TYPE = 0x62;
const char START_CMD = 0x02;

const int SERVER_ENABLED_ADDR = 1;

const int PLM_RESPONSE_LENGTHS[] = {
  11, // 0x50 - INSTEON Standard Message Received
  25, // 0x51 - INSTEON Extended Message Received
  4,  // 0x52 - X10 Received
  10, // 0x53 - ALL-Linking Completed
  3,  // 0x54 - Button Event Report
  2,  // 0x55 - Button Event Report
  7,  // 0x56 - ALL-Link Cleanup Failure Report
  10, // 0x57 - ALL-Link Record Response
  3,  // 0x58 - ALL-Link Cleanup Status Report
  0,  // 0x59 - not used
  0,  // 0x5A - not used
  0,  // 0x5B - not used
  0,  // 0x5C - not used
  0,  // 0x5D - not used
  0,  // 0x5E - not used
  0,  // 0x5F - not used
  9,  // 0x60 - Get IM Info
  6,  // 0x61 - Send ALL-Link Command
  9,  // 0x62 - Send INSTEON Message (23 for extended)
  5,  // 0x63 - Send X10
  5,  // 0x64 - Start ALL-Linking
  3,  // 0x65 - Cancel ALL-Linking
  6,  // 0x66 - Set Host Device Category
  3,  // 0x67 - Reset the IM
  4,  // 0x68 - Set INSTEON ACK Message Byte
  3,  // 0x69 - Get First ALL-Link Record
  3,  // 0x6A - Get Next ALL-Link Record
  4,  // 0x6B - Set IM Configuration
  3,  // 0x6C - Get ALL-Link Record for Sender
  3,  // 0x6D - LED On
  3,  // 0x6E - LED Off
  12, // 0x6F - Manage ALL-Link Record
  4,  // 0x70 - Set INSTEON NAK Message Byte
  5,  // 0x71 - Set INSTEON ACK Message Two Bytes
  3,  // 0x72 - RF Sleep
  6   // 0x73 - Get IM Configuration
};

const int PLM_REQUEST_LENGTHS[] = {
  2,  // 0x60 - Get IM Info
  5,  // 0x61 - Send ALL-Link Command
  8,  // 0x62 - Send INSTEON Message (22 for extended)
  4,  // 0x63 - Send X10
  4,  // 0x64 - Start ALL-Linking
  2,  // 0x65 - Cancel ALL-Linking
  5,  // 0x66 - Set Host Device Category
  2,  // 0x67 - Reset the IM
  3,  // 0x68 - Set INSTEON ACK Message Byte
  2,  // 0x69 - Get First ALL-Link Record
  2,  // 0x6A - Get Next ALL-Link Record
  3,  // 0x6B - Set IM Configuration
  2,  // 0x6C - Get ALL-Link Record for Sender
  2,  // 0x6D - LED On
  2,  // 0x6E - LED Off
  11, // 0x6F - Manage ALL-Link Record
  3,  // 0x70 - Set INSTEON NAK Message Byte
  4,  // 0x71 - Set INSTEON ACK Message Two Bytes
  2,  // 0x72 - RF Sleep
  2   // 0x73 - Get IM Configuration
};

/*TCPServer plmServer = TCPServer(9761);
TCPClient plmClient;*/

uint8_t commandBuffer[25];
int commandLength = 0;
int commandCounter = 0;
uint8_t commandType = 0;
unsigned long lastByteTime;

uint8_t serverEnabled = 0;

void setup() {
  Spark.function("insteon", insteonCommand);
  Spark.function("config", config);
/*
  serverEnabled = EEPROM.read(SERVER_ENABLED_ADDR);

  if(serverEnabled == 0xFF) {
    serverEnabled = 0;
    EEPROM.write(SERVER_ENABLED_ADDR, serverEnabled);
  }

  if(serverEnabled) {
    plmServer.begin();
  }*/
  Serial1.begin(19200);
}

void loop() {
  /*if(serverEnabled) {
    if (plmClient.connected()) {
      while (plmClient.available()) {
        char toPLMByte = plmClient.read();
        Serial1.write(toPLMByte);
      }
    } else {
      plmClient = plmServer.available();
    }
  }*/
  while(Serial1.available()) {
    char fromPLMByte = Serial1.read();
    processPLMCommand(fromPLMByte);
  }

}

int config(String args) {

  if(args.indexOf("plm=true") >= 0) {
    EEPROM.write(SERVER_ENABLED_ADDR, 1);
  } else if(args.indexOf("plm=false") >= 0) {
    EEPROM.write(SERVER_ENABLED_ADDR, 0);
  }

  return 1;
}

int insteonCommand(String args) {
  bool validCommand = false;

  unsigned int argsLen = args.length();

  if(argsLen % 2 != 0 ) {
    return -1;
  }

  int cmdLength = argsLen / 2;
  if(cmdLength < 2) {
    return -2;
  }

  uint8_t cmd[cmdLength];


  for(int i = 0; i < cmdLength; i++) {
    int j = 2*i;
    uint8_t val1 = hexToInt(args.charAt(j++));
    uint8_t val2 = hexToInt(args.charAt(j));

    if(val1 > 0x0F || val2> 0x0F) {
      return -3;
    }

    uint8_t val = (val1 << 4) | val2;
    cmd[i] = val;
  }

  if(cmd[0] != START_CMD) {
    return -4;
  }

  if(cmd[1] < 0x60 || cmd[1] > 0x73) {
    return -5;
  }

  int expectedLength = PLM_REQUEST_LENGTHS[cmd[1] - 0x60];
  if(cmd[1] == 0x62 && cmdLength >= 6 && cmd[5] & 0x10) {
    expectedLength = 22;
  }

  if(expectedLength != cmdLength) {
    return -6;
  }


  return Serial1.write(cmd, cmdLength);
}

uint8_t hexToInt(char hex) {
  if(hex >= '0' && hex <= '9') {
    return hex - '0';
  }

  if(hex >= 'A' && hex <= 'F') {
    return hex - 'A' + 10;
  }

  if(hex >= 'a' && hex <= 'f') {
    return hex - 'a' + 10;
  }

  return 0xFF;
}

void processPLMCommand(char byte) {
  commandBuffer[commandCounter++] = byte;

  if(commandCounter > 1) {

    unsigned long currentTime = millis();
    if(currentTime - lastByteTime < 1000) { // timeout after 1000 ms
      lastByteTime = currentTime;
      if(commandCounter == 2) {
        commandType = byte;
        commandLength = PLM_RESPONSE_LENGTHS[byte - 0x50];
      }

      if(commandType == SEND_MSG_TYPE && commandCounter == 6 && byte & 0x10) {
        commandLength = 23;
      }

      if(commandCounter == commandLength) {
        commandCounter = 0;
        /*if (serverEnabled && plmClient.connected()) {
          plmServer.write(commandBuffer, commandLength);
        }*/

        String commandStr = "";

        for(int i = 0; i < commandLength; i++) {
          if(commandBuffer[i] <= 0x0f) {
            commandStr += "0";
          }
          commandStr += String(commandBuffer[i], HEX);
        }

        Spark.publish("insteon", commandStr, 60, PRIVATE);
      }
    } else {
      commandCounter = 0;
    }

  } else if(byte == START_CMD) {
    lastByteTime = millis();
  } else {
    commandCounter = 0;
  }
}
