#ifndef __MENU_H__
#define __MENU_H__

namespace xistor
{

void ShowMainMenuBar();
void ShowAppSetting(ImGuiStyle*);
void ShowFontSelector(const char* label);
bool ShowStyleSelector(const char* label);

// color style
void StyleColorsDark2();
void StyleColorsEasy();

}   // namespace xistor
#endif // __MENU_H__