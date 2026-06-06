# **Title: Multi-level-collation algorithm**



## Abstract



This informal and incomplete document tries to justify and convince the need for a multi-level-collation algorithm interface in C++, by outlining the motivations, and providing an reference implementation, to show the areas of implementation-defined optimization if such an interface is standardized. 



## The notion of a Multi-layer collation algorithm (1)



### Introduction (1.1)



To understand the need of a collation algorithm, lets supposed you have eight strings:



"192.168.18.1/24"



"192.168.18.2/28"







"172.16.18.1/28"



"172.16.18.2/24"







"fe80::1/64"



"fe80:1fff::1/64"







Now if I want the order to be such that, the ones  with a higher subnet mask (the number after /) to come after the ones with a lower one, and to make ipv6 addresses(the ones in the end) to come before the rest, hence the hierarchy of rules could be defined as:



subnet ID,



IP version,



IP number,



equivalent classes (raw byte comparisons on the encoded values)



according to this hierarchy:



"172.16.18.2/24"



"192.168.18.1/24"



"192.168.18.2/28"



"fe80::1/64"



"fe80:1f::1/64"



In this example, we can remove the IP version part comparison, since private ipv6 networks have higher subnet addresses anyway, but because since the main goal is to explain this topic well, lets bear with it anyway.



As one can see, the motivation is to be able to specify the order of strings based on arbitrary rules. 



### Steps to do it yourself  (1.2)



The first step to implement such a thing is to assign every digit itself a sort key. To allow for such sort keys to support our 3 level multi-level collation, we need a scheme, such that each character has a three byte sort key, and at as the level goes down, the first few respective bytes turn to zero. For example, we could assign the following characters keys from 0-0xFF FF FF:



A, B, C



Then the keys 0-0x00 FF FF to:



D E F



Then the keys 0-0x00 00 FF to: 



G H I



Then Assign the rest characters the value:



0x00 00 00



For a string with N elements, the collation key for the whole string will become:



first\_byte\_of\_the\_first\_element...first\_byte\_of\_the\_N-1\_element 00 second\_byte\_of\_the\_first\_element...second\_byte\_of\_the\_N-1\_element 00 third\_byte\_of\_the\_first\_element...third\_byte\_of\_the\_N-1\_element



If any byte is 00, it must be omitted, aside from the ones explicitly mentioned it the list as "00".



There is a issue though, that our example in 1.1 is actually based on context(of the whole string) rather than just the characters themselves, hence our algorithm requires a helper algorithm that maps characters after a specified delimiter (in our case "/") to something else, which in our case is something to be at the top tier (have the widest possible numeric range). The other context based check that we have is defining the difference between ipv4 and ipv6, such a difference could again be defined by the encounter of a specified character or a simple regex expression. This leaves us with the need of two functions for encoding strings into different things based on context:



One relies on the presence of a range of characters (or a delimiter)



The second relies on a regex expression.



in our case, since each character would fit into 1 byte, even with additional context based mapping, since ipv4 representations normally are in the base 10 form of 4 bytes, Where as ipv6 is in base 16, furthermore, the rest of the characters (":", "/", "."), are all assigned the value 0.



## Interface of the proposed Data Structures and Algorithms (2)

### Interface (2.1)



The first goal is to defined every layer and the characters at each one, for that we want to construct a collation table:



template<typename T, std::integral E>



struct collation\_table{



requires(T<T<E>> a, T<E> b, E c){



a.end();



a.begin()++;



a.size();



a=a;



decltype(a) d= a;







b.end();



b.begin()++;



b.size();



b=b;



decltype(b) e= a;



b\\\[0];//allow for possible binary searches







c=c;



decltype(c) f= c;



c<=>c;//to allow simple optimizations on the infrangible integral element type.



}



collation\_table(T<T<E>>);







};



The every element of T<T<E>> must be a T<E> with sorted elements.



The collation algorithm is defined as:



template<typename T, std::integral E>



collation(collation\_table<T, E>, T<T<E>> list\_to\_be\_collated);



We must also define a helper encoding algorithm to translate characters based on context:





































