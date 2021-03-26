#ifndef INIFILE_PARSER_H
#define INIFILE_PARSER_H

#define MAX_LINE_LENGTH    512 


int iniFileOpen(const char* filename);
int iniFileClose();

int iniFileGetInt(const char* section, 
                  const char* entry, 
                  int   def);
int iniFileGetString(const char* section, 
                     const char* entry, 
                     const char* defVal, 
                     char* buffer, 
                     int   bufferLen);
int iniFileGetSection(const char* section, 
                     const char* buffer, 
                     int   bufferLen);
int iniFileWriteInt(const char* section, 
                    const char* entry, 
                    int value);
int iniFileWriteString(const char* section, 
                       const char* entry, 
                       const char* buffer);
int iniFileWriteSection(const char* section, 
                        const char* buffer);


#endif
