#include "PortDistributor.h"

#include <cassert>
#include <Windows.h>
#include <ConsoleLib/Console.h>

int WINAPI wWinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPWSTR, _In_ int) {
  Console::GetInstance()->RedirectStdHandles();

  

  Console::GetInstance()->Pause();
  return 0;
}
