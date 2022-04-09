#include<iostream>
#include<vector>
#include<string>

using namespace std;

int main() {
    string sMailBodyFile = "";
    
    string sMsg = "\nErrors while updating cost data for the following part number(s):\n\n";
	string sErpSysName = "\n Part Number\n";
	string sLine = "\n-----------------------------------------------------\n";
    
   std::vector<string> a = {"Part1","part2","part3","part4","part5"};
   
   string mailBody = "";
   
      for(int i=0; i < a.size(); i++){
          
         // mailBody =mailBody.append(a.at(i));
         mailBody = mailBody+"\n"+a.at(i);
      }
      sMailBodyFile = sMsg+sErpSysName+sLine+mailBody+sLine;
      std::cout <<sMailBodyFile  << endl;
      
  
   return 0;
}