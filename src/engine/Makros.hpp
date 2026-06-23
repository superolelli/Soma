#pragma once




#define SAFE_DELETE(X) {if(X != nullptr) {delete(X); X=nullptr;}}
