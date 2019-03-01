/** \brief test of ahfits::ahFitsClone
    \author Mike Witthoeft
    \date 2012-06-13
*/

#include "ahfits/ahfits.h"

#include <iostream>


int main() {

std::string src="ae400015010.hk";
std::string dest="clone.hk";

ahfits::ahFitsClone(src,dest,false);

return 0;

}
