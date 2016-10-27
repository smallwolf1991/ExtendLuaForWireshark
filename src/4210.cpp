#include <xlib.h>

#include "lua.hpp"

#include <fstream>
#include <iostream>

using namespace std;

//����wireshark����init.lua����������consloe.luaʱ��Ƕ����ػ���
extern "C" void load_lua_plugins(lua_State* ls)
  {
  static bool lua_plugins_loaded = false;
  if(lua_plugins_loaded)  return;

  lua_plugins_loaded = true;

  const auto oldtop = lua_gettop(ls);

  lua_getglobal(ls, "Proto");           //�ж��Ƿ���wireshark������
  if(lua_type(ls, -1) == LUA_TNIL)
    {
    lua_settop(ls, oldtop);
    return;
    }
  lua_settop(ls, oldtop);

  HMODULE hmod = nullptr;
  GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCTSTR)load_lua_plugins, &hmod);
  char name[MAX_PATH];
  GetModuleFileNameA(hmod, name, sizeof(name));

  char drive[_MAX_DRIVE];
  char dir[_MAX_DIR];
  char fname[_MAX_FNAME];
  char ext[_MAX_EXT];

  auto en = _splitpath_s(name, drive, dir, fname, ext);
  if(0 != en) return;                   //·������ʧ��

  const string path(string(drive) + string(dir) + "LuaPlugins\\");

  //���Ӽ���·��
  lua_settop(ls, oldtop);
  xmsg cmd;
  cmd << "package.path = package.path .. [[;" << path << "?.lua]];";
  luaL_dostring(ls, cmd.c_str());
  lua_settop(ls, oldtop);

  const string fn(path + "*.luae");

  WIN32_FIND_DATAA fd;
  HANDLE hf = FindFirstFileA(fn.c_str(), &fd);
  if(hf == INVALID_HANDLE_VALUE)        //�ļ�����ʧ��
    return;
  do 
    {
    const string ff(path + fd.cFileName);
    lua_settop(ls, oldtop);

    if(LUA_OK != luaL_loadfile(ls, ff.c_str()) || LUA_OK != lua_pcall(ls, 0, LUA_MULTRET, 0))
      {
      lua_error(ls);
      }

    lua_settop(ls, oldtop);
    }while(FindNextFileA(hf, &fd));
  FindClose(hf);
  }

/*
lauxlib.c
  luaL_loadfilex      

linit.c
  loadedlibs          ���xlualib��ʼ��

lstrlib.c             ���string.pack��string.unpack���ܡ���һЩ��Ӧ������

luaconf.h
  LUA_UNSIGNED        �޸�ʹpack��unpack��ȷ

*/