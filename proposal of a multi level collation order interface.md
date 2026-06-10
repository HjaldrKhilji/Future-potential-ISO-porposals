# **Title: Multi-level-collation algorithm**



## Abstract



This informal and incomplete document tries to justify and convince the need for a multi-level-collation algorithm interface in C++, by outlining the motivations, and providing an reference implementation, to show the areas of implementation-defined optimization if such an interface is standardized. It also expanded the notion of collation to include context.



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



There is a issue though, that our example in 1.1 is actually based on context(of the whole string) rather than just the characters themselves, hence our algorithm requires a helper algorithm that maps characters after a specified delimiter (in our case "/") to something else, which in our case is something to be at the top tier (have the widest possible numeric range). The other context based check that we have is defining the difference between ipv4 and ipv6, such a difference could again be defined by the encounter of a specified character or a simple regex expression. This leaves us to rely on processes regex expressions the whole time, such regex expression could in fact be a simple character.







in our case, since each character would fit into 1 byte, even with additional context based mapping, since ipv4 representations normally are in the base 10 form of 4 bytes, Where as ipv6 is in base 16, we would use 1 bytes per level, furthermore, the rest of the characters (":", "/", "."), are all assigned the value 0, which further makes it easier to fit them. Since its better to store a processed regex expression once and use it again and again, then to using a function to process a regex expression every time, we would do the former, hence there would be one function instead that will only allow us to rely on a structure called



## Interface of the proposed Data Structures and Algorithms (2)

### Interface (2.1)



The first goal is to defined every layer and the characters at each one, for that we want to construct a collation table:



template<template<typename U> T, std::integral E, template<typename Y> Inner=T> // inner is useful if the size is a part of the type specialization. In anycase, the size of the inner most dimension should be equal to the size of the outer most.

requires(T<T<E>> a, T<E> b, E c) {



&#x09;a.end();



&#x09;a.begin()++;



&#x09;a.size();



&#x09;a=a;



&#x09;decltype(a) d= a;







&#x09;b.end();



&#x09;b.begin()++;



&#x09;b.size();



&#x09;b=b;



&#x09;decltype(b) e= a;



&#x09;b\[0];//allow for possible binary searches







&#x09;c=c;



&#x09;decltype(c) f= c;



&#x09;c<=>c;//to allow simple optimizations on the infrangible integral element type.



}



struct collation\_table {



&#x09;constexpr collation\_table\& encode\_regex\_into\_table(std::string\& regex\_expression);

&#x09;constexpr collation\_table\& encode\_regex\_into\_table(std::string\&\& regex\_expression);

&#x09;//avoids std::regex to avoid inefficiency, so the implementation can use alternative techniques.



&#x09;constexpr collation\_table\& change\_collation\_table(T<T<E>>\&);



&#x09;constexpr collation\_table\& change\_collation\_table(T<T<E>>\&\&);





&#x09;constexpr  collation\_table(T<T<E>>\&);

&#x09;constexpr  collation\_table(T<T<E>>\&\&);



&#x09;constexpr collation\_table\& operator=(collation\_table\&);

&#x09;constexpr collation\_table\& operator=(collation\_table\&\&);



&#x09;//our main collation algorithm

&#x09;template<typename U= T>//useful in case, T is a non dynamic container whose size is encoded in its size, in which case, you must be extra Extra careful.

&#x09;constexpr U<E> prepare\_for\_collation(T<E>\& list\_to\_be\_processed);//prepares for collation, such that two of the ones processed by this function could be collated by simple using "==" operator.

&#x09;template < typename U = T>// useful in case, T is a non dynamic container whose size is encoded in its size, in which case, you must be extra Extra careful.

&#x09;constexpr bool collate(T<E>\& a, T<E>\& b);//collates between two elements, it could be faster for one of collations because regex groups don't have to be stored, but rather compared one by one, with the ones before getting a higher precedence (basically lexicographically sorting).





};



The every element of T<T<E>> must be a T<E> with sorted elements to allow for certain theoretical optimization. The end of T<E> is going to be signaled by an out of order element showing up. A user can in fact define a certain type T such that the combination T<T> returns in a flat map template, however, it is obvious that the standard cannot provide any help in guiding the user towards that other than advice or suggestions by the S20 working group.

### 

### Usage (2.2)

The first thing that you notice is that the collation table must be processed first, then regex will do the encoding of characters to differentiate them from the same ones that are found in some other context. lets take example:

say T<T<E>> used to construct the collation table is:

std::vector<std::vector<char>>, and the content are so follows (to complement our example in ***1.1***:

{{1,2,3,4,5,6,7,8,9,a,b,c,d,e,f}}, all of them are gonna be given internal sort keys, while the characters not mentioned will have the sort key 0, that is that they wont be considered at all when encountered in a string. Say our regex is as follows:

note that I used multiple lines to simplify expression.

( \[(?: (?: \[\[digit]] | \[abcdef] : ){0, 4} ) {0,8}

&#x20; (?: (?: \[\[digit]] | \[abcdef] : ){0, 4} ) {0,6} :: (?: \[\[digit]] | \[abcdef] ){0, 4} (?:) ] )

( \[ (?: (?: \[\[digit]] . ){0, 3} ) {4}  (?:) ] )

(\[/\[\[digits]]{1,2}])

Each group captured, will be encoded into a lower level, hence the desired hirearchy in ***1.1*** comes into existence. Once such encoding happens internally, the table original collation table stored is expanded to account for the new ones:



{{1,2,3,4,5,6,7,8,9,a,b,c,d,e,f}, {encoded versions of the values in first list, but with lower sort key precedence}, {encoded versions of the values in first list, but with lower sort key precedence}}



Such a internal table is of course conceptual and it can be optimized away, by simply giving the collation of upper group matches a precedence over the lower ones.







## Reference Implementation (3)

## Introduction (3.1)

This section provides a reference implementation for implementing the collation\_table type. It is to be noted that such an implementation is purely conceptual and provides zero exception safeties or details on weather such a type is supposed to be standard layout, trivially copyable, etc. This will hopefully cement the notion of this collation interface with an usable implementation into a topic of community interest.



### Implementing the core mechanism (3.2)

The main notion of collation is to identify characters based on characters, hence we must implement that before implementing the regex encoding mechanism. For this reason, in this section, we assume that the regex expression passed by the user is empty. To keep it short, we will also avoid including the requires statement.

Also note that the example below is slow since it relies on std::regex rather than better alternatives.



template<template<typename U> T, std::integral E, template<typename Y> Inner=T>

struct collation\_table {



&#x20;       T < T < E >> internal\_table;

&#x20;       T < T < Inner < E >>> internal\_sort\_keys; // the representation could of course be optimized by strictly using non dynamic storage but that would reduce the flexibility of changing the collation table later. I believe a provision for statement metaprogramming could help these issues. Amount of elements in lists that is of the lower dimension of internal\_sort\_keysis equal to the rows in internal\_table, which in turn is equal to  "internal\_sort\_keys.size()".

&#x20;       std::regex internal\_regex;



&#x20;       constexpr collation\_table \& encode\_regex\_into\_table(std::string \& regex\_a) {

&#x20;               internal\_regex = a;

&#x20;               return \* this;



&#x20;       }

&#x20;       constexpr collation\_table \& encode\_regex\_into\_table(std::string \&\& a) {

&#x20;               internal\_regex = std::move(a);

&#x20;               return \* this;

&#x20;       }



&#x20;       constexpr collation\_table \& change\_collation\_table(T < T < E >> \& a) {

&#x20;               internal\_table = a;

&#x20;               return \* this;

&#x20;       }



&#x20;       constexpr collation\_table \& change\_collation\_table(T < T < E >> \&\& a) {

&#x20;               internal\_table = std::move(a);

&#x20;               return \* this;

&#x20;       }



&#x20;       constexpr collation\_table(T < T < E >> \& a): internal\_table {

&#x20;               a

&#x20;       } {}

&#x20;       constexpr collation\_table(T < T < E >> \&\& a): internal\_table {

&#x20;               std::move(a)

&#x20;       } {}



&#x20;       constexpr collation\_table \& operator = (collation\_table \& ) =

&#x20;               default;

&#x20;       constexpr collation\_table \& operator = (collation\_table \&\& ) =

&#x20;               default;



&#x20;       template < typename U = T >

&#x20;               constexpr U < E > prepare\_for\_collation(T < E > \& list\_to\_be\_processed) {

&#x20;                       //assuming the internal regex expression is not there, therefor skipping it:

&#x20;                       std::size\_t size\_of\_result = 0;

&#x20;                       U < E > result;

&#x20;                       if constexpr(std::is\_user\_declared < ^ ^ U < E >::reserve > || std::is\_user\_declared < ^ ^ U < E >::push\_back > || std::is\_user\_declared < ^ ^ U < E >::push\_front > ) {

&#x20;                               for (auto x: list\_to\_be\_collated) {

&#x20;                                       size\_of\_result += (std::ranges::find(internal\_table, x) != std::end(internal\_table)) ? 1 : 0;

&#x20;                               }

&#x20;                               if (std::is\_user\_declared < ^ ^  U < E >::reserve > ) {

&#x20;                                       result.reserve(list\_to\_be\_processed.size()); //just to show case the possible room of optimizations

&#x20;                               }

&#x20;                       }

&#x20;                       for (auto y: list\_to\_be\_collated) {

&#x20;                               for (auto x: y) {

&#x20;                                       auto iter = std::ranges::find(internal\_table, x);

&#x20;                                       if (iter != std::end()) {

&#x20;                                               //if this condition isnt true then nothing happens since characters not registered in the collation table are ignored.

&#x20;                                               if constexpr(std::is\_user\_declared < ^ ^ U < E >::push\_back > || std::is\_user\_declared < ^ ^ U < E >::push\_front > ) {

&#x20;                                                       //I think dynamic non contagious containers should not be supported (bad idea to support them), as shown below:

&#x20;                                                       std::vector < E > temp(internal\_table.size() \* size\_of\_result);

&#x20;                                                       for (E individual: \* std::next(std::begin(internal\_sort\_keys), iter - std::begin(list\_to\_be\_collated))) {

&#x20;                                                               temp.push(individual);

&#x20;                                                       }

&#x20;                                                       if constexpr(std::is\_user\_declared < ^ ^ U < E >::push\_back > ) {

&#x20;                                                               for (std::size\_t i = 0; i < internal\_table.size(); i++) {

&#x20;                                                                       for (std::size\_t j = i; j < size\_of\_result; j += 4) {

&#x20;                                                                               result.push\_back(temp\[j]);

&#x20;                                                                       }

&#x20;                                                               }

&#x20;                                                       } else {

&#x20;

&#x20;                                                                       for (std::size\_t i = internal\_table.size(); i < 0; i--) {

&#x20;                                                                               for (std::size\_t j = size\_of\_result; j < i; j -= 4) {

&#x20;                                                                                       result.push\_front(temp\[j]);

&#x20;                                                                               }

&#x20;                                                                       }

&#x20;



&#x20;                                                       }



&#x20;                                               }

&#x20;                                       } else {

&#x09;					auto\& source= \* std::next(std::begin(internal\_sort\_keys), iter - std::begin(list\_to\_be\_collated));

&#x20;                                               for (E individual=source.begin(), auto iter=result.begin(); individual<source.end(); individual++, iter++) {

&#x20;                                                       \*iter= \*individual;

&#x09;						iter++;

&#x20;                                               }



&#x20;                                       }



&#x20;                               }



&#x20;                       }

&#x20;               }

&#x20;       return result;

}

template < typename U = T>

constexpr std::weak\_ordering collate(T < E > \& a, T < E > \& b){

&#x09;return prepare\_collation(a) <=> prepare\_collation(b);

&#x09;}

&#x09;

};

### The missing pieces (3.3)

One could easily point out that the regex based collation has no facilities as the above implementation ignored it completely in favor of showcasing the main idea's implementation. The second thing is that the member internal\_sort\_keys isn't initialized properly, which while making the example easier to understand, is to be completed.  One might however note that using both  internal\_sort\_keys and internal\_table hurts cache locality for every obvious reasons. We now know that in the next section we must complete our implementation by:

1.By making use of internal\_regex

2.By making use of internal\_sort\_keys

3.By avoiding the use of internal\_table completely, and to instead use internal\_sort\_keys only, to help with cache locality. Such a replacement would require some extra processor usage but will still be more efficient than continuously changing in the usage of two containers.

### filling in the whole piece (3.4)

The following example would a be a full-fledged implementation that is to be as general as possible. The proposal does not expect the usage of any of the concepts denoted by the "requires block" and nor does it expect the usage of std::regex internally, it does however hint by the "requires block" the requirement of certain member functions.

namespace collation\_impl\_helpers{

template<template<typename U> T, std::integral E, template<typename Y> Inner=T>

T < Inner < E >> Set\_up\_row\_in\_a\_sort\_key\_table(T < E > character\_table\_row){

&#x09;if(std::is\_user\_declared < ^ ^ T < E> >::push\_back >){

&#x09;for(auto current\_outer\_dimension= character\_table.begin(); current\_outer\_dimension<character\_table.end(); current\_outer\_dimension++){

&#x09;	

&#x09;}

&#x09;}

&#x09;//***todo***

}

template<template<typename U> T, std::integral E, template<typename Y> Inner=T>

T < T < Inner < E >>> Set\_up\_sort\_keys(T < T < E >> character\_table){

&#x09;T < T < Inner < E >>> result;

&#x09;if constexpr(std::is\_user\_declared < ^ ^ T < T < Inner < E >>>::reserve >){

&#x09;	result.reserve(character\_table.size());

&#x09;	//am not going to reserve space for columns as my untested assumption is that it would ruin cache locality on most systems. I will however not test such assumptions since this is a reference implementation and my job is not to implement it in detail to fit every architecture properly, as such jobs are for implementations.

&#x09;}

&#x09;if constexpr(std::is\_user\_declared < ^ ^  T < T < Inner < E >>>::push\_back > || std::is\_user\_declared < ^ ^  T < T < Inner < E >>>::reserve >){



&#x09;for(auto current\_outer\_dimension= character\_table.begin(); current\_outer\_dimension<character\_table.end(); current\_outer\_dimension++){

&#x09;	result.push\_back( T < Inner < E >>{});

&#x09;	result.back()= Set\_up\_row\_in\_a\_sort\_key\_table(\*current\_outer\_dimension);



&#x09;}

&#x09;

&#x09;}

&#x09;if constexpr( std::is\_user\_declared < ^ ^  U < E >::push\_front >){

&#x09;	for(auto current\_outer\_dimension= std::prev(character\_table.end()); true; current\_outer\_dimension++){

&#x09;	if(current\_outer\_dimension== character\_table.begin()){

&#x09;	result.push\_front( T < Inner < E >>{});

&#x09;	result.front()= Set\_up\_row\_in\_a\_sort\_key\_table(\*current\_outer\_dimension);

&#x09;	break;

&#x09;	}

&#x09;	else{

&#x09;	result.push\_front( T < Inner < E >>{});

&#x09;	result.front()= Set\_up\_row\_in\_a\_sort\_key\_table(\*current\_outer\_dimension);

&#x09;	}

&#x09;	



&#x09;}

&#x09;else{

&#x09;//meaning that non dynamic storage is being used

&#x09;for(auto current\_outer\_dimension= character\_table.begin(), auto current\_result\_dimension= result.begin(); current\_outer\_dimension<character\_table.end(); current\_outer\_dimension++, current\_result\_dimension++){

&#x09;	\*current\_result\_dimension= Set\_up\_row\_in\_a\_sort\_key\_table(\*current\_outer\_dimension);

&#x09;}

&#x09;}

&#x09;}

}

template<template<typename U> T, std::integral E, template<typename Y> Inner=T>

requires(T<T<E>> a, T<E> b, E c) {



&#x09;a.end();



&#x09;a.begin()++;



&#x09;a.size();



&#x09;a=a;



&#x09;decltype(a) d= a;







&#x09;b.end();



&#x09;b.begin()++;



&#x09;b.size();



&#x09;b=b;



&#x09;decltype(b) e= a;



&#x09;b\[0];







&#x09;c=c;



&#x09;decltype(c) f= c;



&#x09;c<=>c;



}

struct collation\_table {



&#x20;       T < T < E >> internal\_table;

&#x20;       T < T < Inner < E >>> internal\_sort\_keys; 

&#x20;       std::regex internal\_regex;

&#x20;       constexpr collation\_table \& encode\_regex\_into\_table(std::string \& regex\_a) {

&#x20;               internal\_regex = a;

&#x20;               return \* this;



&#x20;       }

&#x20;       constexpr collation\_table \& encode\_regex\_into\_table(std::string \&\& a) {

&#x20;               internal\_regex = std::move(a);

&#x20;               return \* this;

&#x20;       }



&#x20;       constexpr collation\_table \& change\_collation\_table(T < T < E >> \& a) {

&#x20;               internal\_table = a;

&#x20;               return \* this;

&#x20;       }



&#x20;       constexpr collation\_table \& change\_collation\_table(T < T < E >> \&\& a) {

&#x20;               internal\_table = std::move(a);

&#x20;               return \* this;

&#x20;       }



&#x20;       constexpr collation\_table(T < T < E >> \& a): internal\_table {

&#x20;               a

&#x20;       } {}

&#x20;       constexpr collation\_table(T < T < E >> \&\& a): internal\_table {

&#x20;               std::move(a)

&#x20;       } {}



&#x20;       constexpr collation\_table \& operator = (collation\_table \& ) =

&#x20;               default;

&#x20;       constexpr collation\_table \& operator = (collation\_table \&\& ) =

&#x20;               default;



&#x20;       template < typename U = T >

&#x20;               constexpr U < E > prepare\_for\_collation(T < E > \& list\_to\_be\_processed) {

&#x20;                       std::size\_t size\_of\_result = 0;

&#x20;                       U < E > result;

&#x20;                       if constexpr(std::is\_user\_declared < ^ ^  U < E >::reserve > || std::is\_user\_declared < ^ ^  U < E >::push\_back > || std::is\_user\_declared < ^ ^  U < E >::push\_front >) {

&#x20;                               for (auto x: list\_to\_be\_processed) {

&#x20;                                       size\_of\_result += (std::ranges::find(internal\_table, x) != std::end(internal\_table)) ? 1 : 0;

&#x20;                               }

&#x20;                               if (std::is\_user\_declared < ^ ^  U < E >::reserve > ) {

&#x20;                                       result.reserve(size\_of\_result\*internal\_table.size()); //just to show case the possible room of optimizations

&#x20;                               }

&#x20;                       }

&#x20;                       for (auto y: list\_to\_be\_collated) {

&#x20;                               for (auto x: y) {

&#x20;                                       auto iter = std::ranges::find(internal\_table, x);

&#x20;                                       if (iter != std::end()) {

&#x20;                                               if constexpr(std::is\_user\_declared < ^ ^  U < E >::push\_back > || std::is\_user\_declared < ^ ^  U < E >::push\_front > ) {

&#x20;                                                       //I think dynamic non contagious containers should not be supported (bad idea to support them), as shown below:

&#x20;                                                       std::vector < E > temp(internal\_table.size() \* size\_of\_result);

&#x20;                                                       for (E individual: \* std::next(std::begin(internal\_sort\_keys), iter - std::begin(list\_to\_be\_collated))) {

&#x20;                                                               temp.push(individual);

&#x20;                                                       }

&#x20;                                                       if constexpr(std::is\_user\_declared < ^ ^  U < E >::push\_back > ) {

&#x20;                                                               for (std::size\_t i = 0; i < internal\_table.size(); i++) {

&#x20;                                                                       for (std::size\_t j = i; j < size\_of\_result; j += 4) {

&#x20;                                                                               result.push\_back(temp\[j]);

&#x20;                                                                       }

&#x20;                                                               }

&#x20;                                                       } else {

&#x20;

&#x20;                                                                       for (std::size\_t i = internal\_table.size(); i < 0; i--) {

&#x20;                                                                               for (std::size\_t j = size\_of\_result; j < i; j -= 4) {

&#x20;                                                                                       result.push\_front(temp\[j]);

&#x20;                                                                               }

&#x20;                                                                       }

&#x20;



&#x20;                                                       }



&#x20;                                               }

&#x20;                                       } else {

&#x09;					auto\& source= \* std::next(std::begin(internal\_sort\_keys), iter - std::begin(list\_to\_be\_collated));

&#x20;                                               for (E individual=source.begin(), auto iter=result.begin(); individual<source.end(); individual++, iter++) {

&#x20;                                                       \*iter= \*individual;

&#x09;						iter++;

&#x20;                                               }



&#x20;                                       }



&#x20;                               }



&#x20;                       }

&#x20;               }

&#x20;       return result;

}

template < typename U = T>

constexpr std::weak\_ordering collate(T < E > \& a, T < E > \& b){

&#x09;return prepare\_collation(a) <=> prepare\_collation(b);

&#x09;}

&#x09;

};







