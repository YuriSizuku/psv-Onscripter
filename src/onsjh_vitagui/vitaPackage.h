#pragma once

#include <vitasdk.h>
#include <iostream>
#define PACKAGE_TEMP_FOLDER std::string("ux0:/temp/pkg/")
#define SFO_SIZE 912

class VitaPackage{
public:
  VitaPackage(const std::string path);

  ~VitaPackage();
  //TITLE=STITLE TITLE_ID
  int SetSFOString(std::string title, std::string title_id);
  int Install();
  //int Install();
  static std::string Package_path;
private:
  std::string path_;
};

