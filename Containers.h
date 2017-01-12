

#pragma once


#include "PreAllocatedNodes.h"



// the preallocated node holder
template <class T> NodeTesseract<T> gNodeTesser;

//
// a simple stack that uses pre-allocated nodes
//
template <class T> class BaseContainer
{
public:

	int size() 
	{
		lock_guard<mutex> bclk(containerMutex);
		return numElements;
	}

protected:

	void setNextPrev(Node<T>* pAhead, Node<T>* pTrail)
	{
		pAhead->pNxt = pTrail;
		pTrail->pPrv = pAhead;
	}

	void addHead(Node<T>* pN)
	{
		numElements++;
		if ( pH && !pT) { 
			pT = pH; 
			setNextPrev(pN, pT);
		}
		else if (pH && pT) 
			setNextPrev(pN, pH);
		pH = pN;
	}

	void addTail(Node<T>* pN)
	{
		numElements++;
		if (pT) {
			setNextPrev(pT, pN);
			if (!pH) pH = pT;
		}
		pT = pN; 
	}

	T removeHead()
	{
		numElements--;
		T t = pH->t;
		pH = gNodeTesser<T>.returnNode(pH, true);
		return t;
	}

	T removeTail()
	{
		numElements--;
		T t = pT->t;
		pT = gNodeTesser<T>.returnNode(pT, false);
		return t;
	}

	int numElements = 0;		// number of T elements on the Stack
	mutex containerMutex;			// our mutex
	Node<T>* pH = nullptr;		// head node
	Node<T>* pT = nullptr;		// tail node
};


//
// a simple stack that uses pre-allocated nodes
//
template <class T> class SimpleStack : public BaseContainer<T>
{
public:

	void push( T t ) {
		lock_guard<mutex> elk(containerMutex);
		Node<T>* pNode = gNodeTesser<T>.getNode();
		pNode->t = t;
		addHead( pNode );
	}

	T pop()	{
		lock_guard<mutex> elk(containerMutex);
		return removeHead();
	}
};



//
// a simple queue that uses pre-allocated nodes
//
template <typename T> class SimpleQueue : public BaseContainer<T>
{
public:

	void push(T t) {
		lock_guard<mutex> bclk(containerMutex);
		Node<T>* pNode = gNodeTesser<T>.getNode();
		pNode->t = t;
		addTail( pNode );
	}

	T pop() {
		lock_guard<mutex> bclk(containerMutex);
		return removeHead();
	}

	void swap(SimpleQueue<T>& sq)
	{
		lock_guard<mutex> bclk(containerMutex);
		while (size())
			sq.push(pop());
	}
};


//
// a simple list that uses pre-allocated nodes
//
template <typename T> class SimpleList : public BaseContainer<T>
{
public:

	void addHead( T t )
	{
		lock_guard<mutex> bclk(containerMutex);
		Node<T>* pNode = gNodesBlocks<T>.getPreAllocNode(t);
		pNode->t = t;
		addHead(t);
	}

	void addTail(T t)
	{
		lock_guard<mutex> bclk(containerMutex);
		Node<T>* pNode = gNodesBlocks<T>.getPreAllocNode(t);
		pNode->t = t;
		addTail(pNode);
	}
	
	T removeHead()
	{
		lock_guard<mutex> bclk(containerMutex);
		return removeHead();
	}

	T removeTail()
	{
		lock_guard<mutex> bclk(containerMutex);
		return removeTail();
	}
};

/*

// a simple list that uses pre-allocated nodes
template <class key, class val> class SimpleMap : public BaseContainer<T>
{
#define NUM_LISTS 8
	NodeTesseract<SimpleList<T>> tesseract;
	
public:

	void add( <key> k, <val> v )
	{
		uint32_t hashPos = hash(k);

		uint8_t ii = (hashPos & 0xFF000000 >> 24);
		uint8_t jj = (hashPos & 0x00FF0000 >> 16);
		uint8_t jj = (hashPos & 0x0000FF00 >> 8);
		uint8_t ll = (hashPos & 0x000000FF);

	}

	void addTail()
	{

	}

	T getHead()
	{

	}

	T getTail()
	{

	}

	void remove(T t)
	{

	}

	T operator [] (int pos)
	{


	}
};




// a simple const array by const we mean once the array
// elements grow there is no shrinking we only null removed
// values so that the array size doesn't change until emptied.
template <typename T> class SimpleArray
{
#define ARRAY_SIZE 256

	template <class T> struct ArrayNode
	{
		T t;
		uint8_t valid = 0;
	};

	ArrayNode<T>* marray[ARRAY_SIZE];
	int maxSize = 0;
	int numElements = 0;
	mutex arrayMutex;		

	void allocSpace( int start ) {
		maxSize *= 2;
		for (int j = start; j < maxSize; ++j)
			marray[j] = new ArrayNode<T>();
	}

public:

	SimpleArray( int size ) {
		maxSize = size;
		allocSpace( 0 );
	}

	~SimpleArray() {
		for (int jj = 0; jj < maxSize; ++jj)
			delete marray[jj];
	}

	int size() {
		return numElements;
	}

	void add( T t) {
		lock_guard<mutex> amlk(arrayMutex);
		if (numElements + 1 >= maxSize)
			allocSpace(maxSize);
		marray[numElements++]->t = t;
		marray[numElements++]->valid = 1;

		SimpleArrayPos sap = dynamic_cast<SimpleArrayPos>(t);
		if (sap) sap->position = (numElements - 1);
	}

	T get(int& index) {
		
		if (index < 0 || index > numElements)
			return nullptr;
		
		lock_guard<mutex> amlk(arrayMutex);
		while ( marray[index++]->valid == 0 ) { 
			// just spin through the invalid Ts
		}
		return marray[index]->t;
	}

	T operator [] (int index) {
		return get(index);
	}
	
	// just mark element as invalid
	void remove( T _t ) {
		
		if (!numElements)
			return;
		
		lock_guard<mutex> amlk(arrayMutex);
		
		SimpleArrayPos* pSAP = dynamic_cast<SimpleArrayPos*>(_t);
		if (pSAP ) {
			marray[pSAP->position]->valid = 0;
			pSAP->position = -1;
		}
	}

	void empty() {
		lock_guard<mutex> amlk(arrayMutex);
		numElements = 0;
	}

	/*
	bool data( T& pT, int start, int& num ){
	if (start <= 0 || start >= numElements )
	lock_guard<mutex> amlk(arrayMutex);
	if ((start + num) >= numElements)
	num = (numElements - start);
	int max = start + num;
	for ( int jj = start, ii = 0; jj < max; jj++, ii++ )
	pT[ii] = marray[jj]->t; return true;

};


/*
#define add_const_reference AddCnstRef

// Variadic tuple type 
template<typename Head, typename... Tail>
class tuple<Head, Tail...> : private tuple<Tail...> 
{

// ... here is the recursion
	// Basically, a tuple stores its head (first (type/value) pair 
	// and derives from the tuple of its tail (the rest of the (type/value) pairs.
	// Note that the type is encoded in the type, not stored as data

	typedef tuple<Tail...> inherited;

public:

	tuple() { }	// default: the empty tuple

	// Construct tuple from separate arguments:
	tuple(typename AddCnstRef<Head>::type v, typename AddCnstRef<Tail>::type... vtail) : m_head(v), inherited(vtail...) { }

	// Construct tuple from another tuple:
	template<typename... VValues> tuple(const tuple<VValues...>& other) : m_head(other.head()), inherited(other.tail()) { }

	template<typename... VValues> tuple& operator=(const tuple<VValues...>& other)	// assignment
	{
		m_head = other.head();
		tail() = other.tail();
		return *this;
	}

	typename add_reference<Head>::type head() {
		return m_head; 
	}
	
	typename add_reference<const Head>::type head() const {
		return m_head; 
	}

	inherited& tail() {
		return *this;
	}
	
	const inherited& tail() const { 
		return *this;
	}

protected:
	
	Head m_head;
};


// perform allcations in another thread ???




/*
// last position for getPosition and operator []
class LastPosition
{
public:
int index = 0;			// last position
Node<T>* pN = nullptr;	// last position node
void reset() { index = 0; pN = nullptr; }
};

LastPosition lp;
T getPosition(int pos)
{
if ( pos == lp )
return getByLP( pos );
else {
lp.reset();
if (pos < (numElements / 2))
return searchFromHead(pos);
else
return searchFromHead(pos);
} }
*/

