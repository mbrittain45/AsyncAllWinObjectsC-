

#pragma once

#include "defines.h"


#ifdef _DEBUG
#define MAX_NODES  16	// 65k = 16^4, 16 ^ 5 = 1M
#else
#define MAX_BLOCKS 32	// 1M = 32^4
#endif


// predefines
template <class T> class NodeArray;
template <class T> class NodeMatrix;
template <class T> class NodeCube;
template <class T> class NodeTesseract;


//
// a double linked node and also used as a single linked node
//
template <class T> class Node
{
public:
	T t;
	Node* pNxt = nullptr;
	Node* pPrv = nullptr;

	~Node() { 
		delete pNxt;
	}
	
	void clear() {
		t = 0; 
		pNxt = nullptr; 
		pPrv = nullptr;
	}
};


//
// array of pre-allocated nodes
//
template <class T> class NodeArray
{
	friend class NodeMatrix<T>;
	friend class NodeTesseract<T>;
	friend class NodeCube<T>;

	Node<T>* pNodes[MAX_NODES];
	int ll = 0;	

public:

	NodeArray() 
	{
		for (int ii = 0; ii < MAX_NODES; ii++)
			pNodes[ii] = new Node<T>();
	}
	
	~NodeArray() 
	{
		for (int ii = 0; ii < MAX_NODES; ii++)
			delete pNodes[ii];
	}
	
	bool getNode(Node<T>*& pN)
	{
		pN = pNodes[ll];
		pNodes[ll] = nullptr;
		if (ll + 1 >= MAX_NODES)
			return true;	// we rolled
		ll++;
		return false;
	}
};



//
// a matrix of pre-allocated nodes
//
template <class T> class NodeMatrix
{
	friend class NodeCube<T>;
	friend class NodeTesseract<T>;

	NodeArray<T>* pNA[MAX_NODES];
	int kk = 0;		// get count

public:

	NodeMatrix()
	{
		for (int ii = 0; ii < MAX_NODES; ii++)
			pNA[ii] = new NodeArray<T>();
	}
	
	~NodeMatrix()
	{
		for (int ii = 0; ii < MAX_NODES; ii++)
			delete pNA[ii];
	}
	
	bool getNode(Node<T>*& pN) 
	{
		if (pNA[kk]->getNode(pN))
		{	
			// the array below us hit its limit
			if (kk + 1 >= MAX_NODES)
				return true;
			kk++;
		}
		return false;
	}
	
	// reposition for a return since we have to move back 1 position
	// but if everyone is on an leaing or trailing edge it becomes
	// a cascading effect backwards in changes.
	bool returnNode( Node<T>* pN )
	{
		if (pNA[kk]->ll - 1 >= 0) {
			pNA[kk]->ll--;
			pNA[kk]->pNodes[pNA[kk]->ll] = pN;
		}
		else if (kk - 1 >= 0) {
			kk--;
			pNA[kk]->pNodes[pNA[kk]->ll] = pN;
		}
		else
			return true; // we have no where to go
		
		return false;
	}
};


//
// a cube of pre-allocated nodes
//
template <class T> class NodeCube
{
	friend class NodeTesseract<T>;

	NodeMatrix<T>* pNM[MAX_NODES];
	int jj = 0;		// get count

public:

	NodeCube() 
	{ 
		for (int ii = 0; ii < MAX_NODES; ii++)
			pNM[ii] = new NodeMatrix<T>();
	}
	
	~NodeCube() 
	{
		for (int ii = 0; ii < MAX_NODES; ii++)
			delete pNM[ii];
	}
	
	bool getNode(Node<T>*& pN ) 
	{
		if ( pNM[jj]->getNode(pN) )
		{
			// the matrix below us hit its limit
			if (jj + 1 >= MAX_NODES)
				return true;	// we rolled
			jj++;
		}
		return false;
	}
	
	bool returnNode(Node<T>* pN)
	{
		if (pNM[jj]->returnNode(pN))
		{	
			// we have to roll back to make room for the node
			if (jj - 1 >= 0) {
				jj--;
				int kk = pNM[jj]->kk;
				pNM[jj]->pNA[kk]->ll++;
				pNM[jj]->returnNode(pN);
			}
			else
				return true; // we have no where to go
		}
		return false;
	}
};



//
// a tesseract of pre-allocated nodes
//
template <class T> class NodeTesseract
{
	NodeCube<T>* pNC[MAX_NODES];
	int ii = 0;	
	bool empty = false;
	bool full = true;

public:

	NodeTesseract() 
	{ 
		for (int ii = 0; ii < MAX_NODES; ii++)
			pNC[ii] = new NodeCube<T>();
	}
	
	~NodeTesseract()
	{
		for (int ii = 0; ii < MAX_NODES; ii++)
			delete pNC[ii];
	}
	
	Node<T>* getNode()
	{
		Node<T>* pN = nullptr;
		
		if (full) full = false;
		if (empty) { assert(0); return pN; }

		if (pNC[ii]->getNode(pN))
		{	
			// the cube below us hit its limit
			if (ii + 1 >= MAX_NODES)
				empty = true;
			else
				ii++;
		}

#ifdef _DEBUG
		int jj = pNC[ii]->jj;
		int kk = pNC[ii]->pNM[jj]->kk;
		int ll = pNC[ii]->pNM[jj]->pNA[kk]->ll;
		TRACE(L"Get Node: %d-%d-%d-%d\n", ii, jj, kk, ll );
#endif

		return pN;
	}

	// return the next or previous node by the flag
	Node<T>* returnNode(Node<T>* pN, bool bRetNext )
	{
		Node<T>* pRN = nullptr;

		if (full) { assert(0); return pRN;}
		if (empty) empty = false;
		
		if (bRetNext)
			pRN = pN->pNxt;
		else
			pRN = pN->pPrv;
		
		pN->clear();
		
		if (pNC[ii]->returnNode(pN))
		{	
			// we have to roll back to make room
			if (ii - 1 >= 0)
				pNC[--ii]->returnNode(pN);
			else
				full = true; // we have to where to go so we're full
		}
		
#ifdef _DEBUG
		int jj = pNC[ii]->jj;
		int kk = pNC[ii]->pNM[jj]->kk;
		int ll = pNC[ii]->pNM[jj]->pNA[kk]->ll;
		TRACE(L"Return Node: %d-%d-%d-%d\n", ii, jj, kk, ll);
#endif
		
		return pRN;
	}
};


