#pragma once
#define SETTING

struct Recive_data { // weigth  from joint divide into 4 area

  float knee1;
  float knee2;
  float knee3;
  float knee4;

};

enum  Theme     { light, dark };
enum  Language  { thai, english };
enum  Modify    { gram, newton, kilogram };

struct Setting_data {
  Theme    theme    = dark;      
  Language language = english;   
  Modify   metric   = gram; 

  bool     isauto   = true;
};
