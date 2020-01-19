#define serialDebug Serial1

void initSerialDebug(){
  serialDebug.begin(9600);
}

void print(String s){
  serialDebug.println(s);
}

void print(String s1, String s2){
  serialDebug.print(s1);
  serialDebug.print(" ");
  serialDebug.println(s2);
}

