#ifndef _SHELL_HPP_
#define _SHELL_HPP_

#include <Kernel.hpp>

namespace Shell {
    struct Commands {
        char *CommandName;
        char *Instruction;
        unsigned long EntryPoint;
    };
    class CommandHistory {
        friend class ShellSystem;
        public:
            inline void Initialize(void) {
                CurrentHistoryCount = 0;
                Index = -1;
            }
            inline void Add(const char *New) {
                Index++;
                Histories[Index] = (char *)MemoryManagement::Allocate(strlen(New)+1);
                strcpy(Histories[Index] , New);
                CurrentHistoryCount++;
            }
            inline void Up(void) {
                if(Index <= 0) {
                    return;
                }
                Index--;
            }
            inline void Down(void) {
                if(Index >= CurrentHistoryCount-1) {
                    return;
                }
                Index++;
            }
            inline void View(char *Data) {
                if(Index == -1) {
                    return;
                }
                strcpy(Data , Histories[Index]);
            }
        private:
            char *Histories[4096];
            int CurrentHistoryCount;
            int Index;
    };
    struct ClipboardHistory {
        char *String;
        
        inline void Initialize(void) {
            String = (char *)MemoryManagement::Allocate(64);
        }
        inline void Copy(const char *New) {
            MemoryManagement::Free(String);
            String = (char *)MemoryManagement::Allocate(strlen(New));
            strcpy(String , New);
        }
        inline void Paste(char *Data) {
            strcpy(Data , String);
        }
    };
    class CommandListSystem {
        friend class ShellSystem;
        public:
            void Initialize(int MaxCount);
            void AddCommand(const char *Name , const char *HelpMessage , unsigned long EntryPoint);
            unsigned long EntryPoint(char *Name);
            inline int GetCommandsCount(void) {
                return CommandsCount;
            }
            inline char **GetCommandNameList(void) {
                return CommandNames;
            }
            inline char **GetCommandHelpMessageList(void) {
                return CommandHelpMessages;
            }
        private:
            int CommandsCount = 0;
            int MaxCommandsCount;
            unsigned long *CommandEntryPoint;
            char **CommandNames;
            char **CommandHelpMessages;
    };
    class ShellSystem {
        friend class CommandHistory;
        friend class CommandListSystem;
        public:
            void AddBasicCommands(void);
            void Start(void);
            CommandListSystem CommandList;
        private: // Processing keyboard input & certain functions
            void ProcessKeyboardLeft(void);
            void ProcessKeyboardRight(void);
            void ProcessKeyboardBackspace(void);
            
            void ProcessDefault(int ASCIIData);
            void Insert_ProcessDefault(int ASCIIData);
            void ProcessCommand(void);
            void Select_ProcessKeybordLeft(void);
            void Select_ProcessKeyboardRight(void);
            void RemoveUnnecessarySpaces(void);

            void ChangeDirectory(int SlicedCommandCount , char **SlicedCommand);

            char *Command;
            
            char *CurrentDirectory;
            unsigned int Offset = 0;
            unsigned int MaxOffset = 0;
            int DefaultPositionX;
            int DefaultPositionY;
            class CommandHistory History;
            struct ClipboardHistory Clipboard;
            int InsertKeyPressed = 0;
    };
};

#endif