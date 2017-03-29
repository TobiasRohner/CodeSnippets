#ifndef SKIPLIST_H
#define SKIPLIST_H

#include <cstdlib>
#include <utility>
#include <iterator>
#include <iostream>


#define MAX_LEVEL 8



//POST: Returns a random number from 1 up to MAX_LEVEL
//      With exponentially falling probabilities
char random_lvl()
{
	char level = 1;
	while ((std::rand() % 2) && (level < MAX_LEVEL))
		++level;
	return level;
}



template<typename T>
class SkipList
{
	struct Node
	{
		Node();
		Node(const T& d);
		Node(const T& d, char lvl);
		Node(const Node& other);
		Node(Node&& other);
		~Node();

		Node& operator=(const Node& other);
		Node& operator=(Node&& other);

		void swap(Node& other);

		char level;
		T data;
		Node** next;
	};


public:
	class iterator : public std::iterator<std::input_iterator_tag, T>
	{
	public:
		explicit iterator(Node* node) : current_node(node) {}
		iterator& operator++() { current_node = current_node->next[0]; return *this; }
		iterator operator++(int) { iterator tmp = *this; ++(*this); return tmp; }
		bool operator==(iterator other) const { return current_node == other.current_node; }
		bool operator!=(iterator other) const { return !(*this == other); }
		T& operator*() { return current_node->data; }
		T& operator*() const { return current_node->data; }

	private:
		Node* current_node;
	};


	SkipList();
	SkipList(const SkipList<T>& other);
	SkipList(SkipList<T>&& other);
	~SkipList();

	SkipList<T>& operator=(const SkipList<T>& other);
	SkipList<T>& operator=(SkipList<T>&& other);

	T* find(const T& value) const;
	void insert(const T& value);
	void erase(const T& value);

	iterator begin() const;
	iterator end() const;


private:
	Node* root;

	void swap(SkipList<T>& other);
};




/*
--------------- Node Implementation ---------------
*/

template<typename T>
SkipList<T>::Node::Node()
{
	level = random_lvl();
	next = new Node*[level];
	//Fill next with nullptrs
	for (char lvl = 0; lvl < level; ++lvl)
		next[lvl] = nullptr;
}


template<typename T>
SkipList<T>::Node::Node(const T& d) : data(d)
{
	level = random_lvl();
	next = new Node*[level];
	//Fill next with nullptrs
	for (char lvl = 0; lvl < level; ++lvl)
		next[lvl] = nullptr;
}


template<typename T>
SkipList<T>::Node::Node(const T& d, char lvl) : data(d), level(lvl)
{
	next = new Node*[level];
	//Fill next with nullptrs
	for (char lvl = 0; lvl < level; ++lvl)
		next[lvl] = nullptr;
}


template<typename T>
SkipList<T>::Node::Node(const Node& other)
{
	//Copy the data of the node
	data = other.data;
	level = other.level;
	//Copying the next pointers of the node
	next = new Node*[level];
	for (size_t i = 0; i < level; ++i)
		next[i] = other.next[i];
}


template<typename T>
SkipList<T>::Node::Node(Node&& other)
{
	swap(other);
}


template<typename T>
SkipList<T>::Node::~Node()
{
	if (next) delete next;      //next could be nullptr, if a move constructor was called and no next array has been initialized
}


template<typename T>
typename SkipList<T>::Node& SkipList<T>::Node::operator=(const typename SkipList<T>::Node& other)
{
	Node tmp(other);    //Copy
	swap(tmp);
	return *this;
}


template<typename T>
typename SkipList<T>::Node& SkipList<T>::Node::operator=(typename SkipList<T>::Node&& other)
{
	swap(other);
	return *this;
}


template<typename T>
void SkipList<T>::Node::swap(Node& other)
{
	std::swap(data, other.data);
	std::swap(level, other.level);
	std::swap(next, other.next);
}




/*
--------------- SkipList Implementation ---------------
*/

template<typename T>
SkipList<T>::SkipList()
{
	T root_elem;
	root = new Node(root_elem, MAX_LEVEL);
}


template<typename T>
SkipList<T>::SkipList(const SkipList<T>& other)
{
	//Initialize a new root element
	T root_elem;
	root = new Node(root_elem, MAX_LEVEL);
	//Copy nodes on the first level
	Node* current_node = root;
	for (const auto& n : other) {
		current_node->next[0] = new Node(n);
		current_node = current_node->next[0];
	}
	//Link the higher levels
	Node* to_link[MAX_LEVEL];
	for (char lvl = 0; lvl < MAX_LEVEL; ++lvl)
		to_link[lvl] = root;
	Node* n = root->next[0];
	while (n) {
		for (char lvl = 1; lvl < n->level; ++lvl) {
			to_link[lvl]->next[lvl] = n;
			to_link[lvl] = n;
		}
		n = n->next[0];
	}
}


template<typename T>
SkipList<T>::SkipList(SkipList<T>&& other)
{
	swap(other);
}


template<typename T>
SkipList<T>::~SkipList()
{
	//Iterate through the nodes on the first level and delete them
	Node* current_node = root;
	while (current_node) {
		Node* next = current_node->next[0];
		delete current_node;
		current_node = next;
	}
}


template<typename T>
SkipList<T>& SkipList<T>::operator=(const SkipList<T>& other)
{
	SkipList<T> tmp(other);
	swap(tmp);
	return *this;
}


template<typename T>
SkipList<T>& SkipList<T>::operator=(SkipList<T>&& other)
{
	swap(other);
	return *this;
}


template<typename T>
T* SkipList<T>::find(const T& value) const
{
	//Return nullptr if no node is in the list
	if (!root->next[0])
		return nullptr;
	Node* current_node = root->next[0];
	//Else continue the search
	while (current_node->data < value) {
		char lvl_step = current_node->level;
		while (lvl_step >= 0 && current_node->next[lvl_step] && current_node->next[lvl_step]->data > value)
			--lvl_step;
		if (lvl_step < 0)   //The value was not found
			return nullptr;
		//The value could still be found
		current_node = current_node->next[lvl_step];
	}
	//The node was found
	return &(current_node->data);
}


template<typename T>
void SkipList<T>::insert(const T& value)
{
	//Find the nodes preceeding the node to insert on each level
	Node* nodes_to_relink[MAX_LEVEL];   //An array of the nodes on all levels after which the new value gets inserted
	//Calculate the highest level specially, as it has no possibility of starting from a previously discovered node
	nodes_to_relink[MAX_LEVEL - 1] = root;
	while (nodes_to_relink[MAX_LEVEL - 1] && nodes_to_relink[MAX_LEVEL - 1]->next[MAX_LEVEL - 1] && nodes_to_relink[MAX_LEVEL - 1]->next[MAX_LEVEL - 1]->data < value)
		nodes_to_relink[MAX_LEVEL - 1] = nodes_to_relink[MAX_LEVEL - 1]->next[MAX_LEVEL - 1];
	//Calculate all the other levels
	for (char lvl = MAX_LEVEL - 2; lvl >= 0; --lvl) {
		if (nodes_to_relink[lvl + 1])
			nodes_to_relink[lvl] = nodes_to_relink[lvl + 1];
		else
			nodes_to_relink[lvl] = root->next[lvl];
		while (nodes_to_relink[lvl] && nodes_to_relink[lvl]->next[lvl] && nodes_to_relink[lvl]->next[lvl]->data < value)
			nodes_to_relink[lvl] = nodes_to_relink[lvl]->next[lvl];
	}

	//Inserting a new node
	Node* to_insert = new Node(value);
	for (char lvl = to_insert->level - 1; lvl >= 0; --lvl) {
		if (nodes_to_relink[lvl]) {
			//Rout the node to insert next pointer to the corresponding node
			to_insert->next[lvl] = nodes_to_relink[lvl]->next[lvl];
			//Rerout the next pointer of the preceeding node to point at this node
			nodes_to_relink[lvl]->next[lvl] = to_insert;
		}
		else
			//Rerout the pointer of the root to point at this node
			root->next[lvl] = to_insert;
	}
}


template<typename T>
void SkipList<T>::erase(const T& value)
{
	//Find the nodes preceeding the node to insert on each level
	Node* nodes_to_relink[MAX_LEVEL];   //An array of the nodes on all levels after which the new value gets inserted
	//Calculate the highest level specially, as it has no possibility of starting from a previously discovered node
	nodes_to_relink[MAX_LEVEL - 1] = root;
	while (nodes_to_relink[MAX_LEVEL - 1] && nodes_to_relink[MAX_LEVEL - 1]->next[MAX_LEVEL - 1] && nodes_to_relink[MAX_LEVEL - 1]->next[MAX_LEVEL - 1]->data < value)
		nodes_to_relink[MAX_LEVEL - 1] = nodes_to_relink[MAX_LEVEL - 1]->next[MAX_LEVEL - 1];
	//Calculate all the other levels
	for (char lvl = MAX_LEVEL - 2; lvl >= 0; --lvl) {
		if (nodes_to_relink[lvl + 1])
			nodes_to_relink[lvl] = nodes_to_relink[lvl + 1];
		else
			nodes_to_relink[lvl] = root->next[lvl];
		while (nodes_to_relink[lvl] && nodes_to_relink[lvl]->next[lvl] && nodes_to_relink[lvl]->next[lvl]->data < value)
			nodes_to_relink[lvl] = nodes_to_relink[lvl]->next[lvl];
	}

	//Iteratively delete the following nodes with value
	Node* current_node = nodes_to_relink[0]->next[0];
	while (current_node && current_node->data == value) {
		//Delete the node
		for (char lvl = 0; lvl < current_node->level; ++lvl)
			nodes_to_relink[lvl]->next[lvl] = current_node->next[lvl];
		Node* tmp = current_node->next[0];
		delete current_node;
		current_node = tmp;
	}
}


template<typename T>
typename SkipList<T>::iterator SkipList<T>::begin() const
{
	return iterator(root->next[0]);
}


template<typename T>
typename SkipList<T>::iterator SkipList<T>::end() const
{
	return iterator(nullptr);
}


template<typename T>
void SkipList<T>::swap(SkipList<T>& other)
{
	std::swap(root, other.root);
}




/*
--------------- Miscellaneous ---------------
*/

template<typename T>
std::ostream& operator<<(std::ostream& os, const SkipList<T>& sklist)
{
	for (const auto& v : sklist)
		os << v << " ";
	return os;
}


#endif