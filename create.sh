#!/bin/bash

toUpper() {
echo $1 | tr  "[:lower:]" "[:upper:]"
}

[ $# -le 0 ] && exit 1;

upper=`toUpper $1`
empty="_H__"
macro="__$upper$empty"

echo "#ifndef $macro
#define $macro


class $1 {
public:
	$1();
	~$1();
};	//class $1
	
#endif	//$macro
"> src/$1.h;

echo "#include <$1.h>

$1::$1() {

}


$1::~$1() {

}
"> src/$1.cpp;

