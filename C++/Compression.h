#pragma once

#include <vector>
#include <cstdint>
#include <limits>
#include <queue>
#include <map>
#include <utility>
#include <type_traits>
#include <iostream>
#include <unordered_map>
#include <algorithm>


//Run Length Encoding (RLE)
namespace RLE
{

	//Performs Run Length Encoding (RLE-Compression) on a container of integer scalars
	//Uses the MSB as an indicator for repetitions:
	//	1: The current value is a counter telling how many times the next element should be repeated
	//	0: The current value can be copied as is in the decoded array
	template<typename Iterator>
	std::vector<typename Iterator::value_type> compress(Iterator begin, Iterator end)
	{
		static_assert(std::numeric_limits<typename Iterator::value_type>::is_integer, "Can only compress Integer Datatypes!");

		//Some useful declarations
		using stype = typename Iterator::value_type;
		using utype = typename std::make_unsigned<stype>::type;	//Unsigned version of the iterator value type (Used for counting reoccurances)
		const utype MSB = 1 << (std::numeric_limits<utype>::digits - 1);
		const utype MAX_COUNT = std::numeric_limits<utype>::max() >> 1;

		std::vector<stype> encoded;
		utype count = 1;
		stype last_element = *begin;

		//Compress all but the last element
		for (Iterator it = begin + 1 ; it != end ; ++it) {
			//If the element has to be stored
			if (*it != last_element || count == MAX_COUNT) {
				//Special case for only one occurance
				if (count == 1 && !(last_element & MSB))
					encoded.push_back(last_element);
				else {
					encoded.push_back(MSB | count);
					encoded.push_back(last_element);
				}
				last_element = *it;
				count = 0;
			}
			++count;
		}
		//Add the last element to the compressed vector
		if (count == 1 && !(last_element & MSB))
			encoded.push_back(last_element);
		else {
			encoded.push_back(MSB | count);
			encoded.push_back(last_element);
		}

		return encoded;
	}


	//Decompresses data which was compressed with RLE
	template<typename Iterator>
	std::vector<typename Iterator::value_type> decompress(Iterator begin, Iterator end)
	{
		static_assert(std::numeric_limits<typename Iterator::value_type>::is_integer, "Can only decompress Integer Datatypes!");

		//Some useful declarations
		using stype = typename Iterator::value_type;
		using utype = typename std::make_unsigned<stype>::type;	//Unsigned version of the iterator value type (Used for interpreting count variables)
		const utype MSB = 1 << (std::numeric_limits<utype>::digits - 1);
		const utype MAX_COUNT = std::numeric_limits<utype>::max() >> 1;

		std::vector<stype> encoded;

		Iterator it = begin;
		while (it != end) {
			if (*it & MSB) {		//*it is a counter indicating how many times the next element should be repeated
				utype count = *it - MSB;
				++it;
				encoded.reserve(encoded.size() + count);
				for (size_t i = 0; i < count; ++i)
					encoded.push_back(*it);
			}
			else	//The element can be inserted in the decompressed vector as is
				encoded.push_back(*it);
			++it;
		}

		return encoded;
	}

}



namespace Huffman
{
	// Contains everything the user has not to worry about
	namespace detail
	{
		// A Huffman Tree implementation
		template<typename WordT>
		class HuffmanTree
		{
			// A node of the huffman tree
			struct Node
			{
				// Indicates how often this word / collection of words occurs
				uint64_t frequency;
				// If this is a leaf node, this contains its data
				WordT    data;
				// The left child node
				Node*    left;
				// The right child node
				Node*    right;


				// Construct a node from a frequency and some data.
				// This sets botch children to nullptr
				inline Node(uint64_t f, WordT d) :
					frequency(f),
					data(d),
					left(nullptr),
					right(nullptr)
				{
				}


				// Construct a node from a frequency and its children
				// This leaves the data uninitialized
				inline Node(uint64_t f, Node* l, Node* r) :
					frequency(f),
					data(WordT()),
					left(l),
					right(r)
				{
				}


				// Copy constructor
				inline Node(const Node& other) :
					frequency(other.frequency),
					data(other.data),
					left(other.left != nullptr ? other.left->clone() : nullptr),
					right(other.right != nullptr ? other.right->clone(): nullptr)
				{
				}


				// Move constructor
				inline Node(Node&& other)
				{
					swap(other);
				}


				// Destructor. This deletes the whole branch to prevent dangling pointers
				inline ~Node()
				{
					if (left)  delete left;
					if (right) delete right;
				}


				// Copy assignment
				inline Node& operator=(const Node& other)
				{
					frequency = other.frequency;
					data = other.data;
					left = other.left->clone();
					right = other.right->clone();
					return *this;
				}


				// Move assignment
				inline Node& operator=(Node&& other)
				{
					swap(other);
					return *this;
				}


				// A node is a leaf, if both its children are nullptr
				inline bool isLeaf() const
				{
					return (left == nullptr) && (right == nullptr);
				}


				// Clone a branch of the tree
				inline Node* clone() const
				{
					Node* new_node = new Node(frequency, data);
					new_node->left = left != nullptr ? left->clone() : nullptr;
					new_node->right = right != nullptr ? right->clone() : nullptr;
					return new_node;
				}


				// Swap the contents of this node with another one
				inline void swap(Node& other)
				{
					std::swap(frequency, other.frequency);
					std::swap(data, other.data);
					std::swap(left, other.left);
					std::swap(right, other.right);
				}


				// A struct for comparing node pointers based on the frequency stored in them.
				// This is only used in the std::map container
				struct NodePtrCmpGreater
				{
					inline bool operator()(const Node* a, const Node* b) const
					{
					return a->frequency > b->frequency;
					}
				};
			};

		public:
			// Constructor
			inline HuffmanTree()
			{
			}


			// Copy constructor
			inline HuffmanTree(const HuffmanTree<WordT>& other) :
				root(other.root->clone()),
				code(other.code)
			{
			}


			// Movement constructor
			inline HuffmanTree(HuffmanTree<WordT>&& other)
			{
				swap(other);
			}


			// Destructor
			inline ~HuffmanTree()
			{
				if (root) delete root;
			}


			// Copy assignment
			inline HuffmanTree<WordT>& operator=(const HuffmanTree<WordT>& other)
			{
				root = other.root->clone();
				code = other.code;
				return *this;
			}


			// Move assignment
			inline HuffmanTree<WordT>& operator=(HuffmanTree<WordT>&& other)
			{
				swap(other);
				return *this;
			}


			// Creates a HuffmanTree from uncompressed data
			template<typename Iterator>
			inline static HuffmanTree<WordT> fromUncompressed(Iterator begin, Iterator end)
			{
				static_assert(std::is_same<typename Iterator::value_type, WordT>::value, "Iterator has wrong type");
				HuffmanTree<WordT> tree;
				tree.makeTree(begin, end);
				tree.makeCode();
				return tree;
			}


			// Reconstructs the HuffmanTree stored in the memory pointed to by begin
			// Returns a HuffmanTree instance and an iterator to the start of the actual data
			template<typename Iterator>
			inline static std::pair<HuffmanTree<WordT>, Iterator> fromCompressed(Iterator begin)
			{
			static_assert(std::is_same<typename Iterator::value_type, bool>::value, "Iterator must return bool");
				HuffmanTree<WordT> tree;
				begin = tree.reconstructTree(begin);
				tree.makeCode();
				return std::make_pair(tree, begin);
			}


			// Constructs a tree based on the data in the given range
			template<typename Iterator>
			inline void makeTree(Iterator begin, Iterator end)
			{
				static_assert(std::is_same<typename Iterator::value_type, WordT>::value, "Iterator has wrong type");
				// Insert all words into a min-heap
				auto freq_table = makeFrequencyTable(begin, end);
				std::priority_queue<Node*, std::vector<Node*>, typename Node::NodePtrCmpGreater> heap;
				for (const auto& p : freq_table)
					heap.push(new Node(p.second, p.first));
				// Repeatedly pop two elements, merge them and insert the parent
				while (heap.size() > 1) {
					Node* min1 = heap.top();
					heap.pop();
					Node* min2 = heap.top();
					heap.pop();
					Node* parent = new Node(min1->frequency + min2->frequency, min1, min2);
					heap.push(parent);
				}
				// Set the root of the huffman tree
				if (root) delete root;
				root = heap.top();
				heap.pop();
			}


			// Constructs a code based on the tree generated earlyer
			inline void makeCode()
			{
				makeCode(root, 0, 0);
			}


			// Reconstructs a tree from compressed data.
			// Returns an iterator to the start of the actual compressed data
			template<typename Iterator>
			inline Iterator reconstructTree(Iterator tree)
			{
				static_assert(std::is_same<typename Iterator::value_type, bool>::value, "Iterator must return bool");
				// Cleanup
				if (root) delete root;
				// Reconstruct the tree
				if (*tree) {
					++tree;
					root = new Node(0, 0);
					for (int i = bits_per_word - 1 ; i >= 0 ; --i) {
					root->data |= static_cast<WordT>(*tree) << i;
					++tree;
					}
				}
				else {
					++tree;
					root = new Node(0, 0);
					tree = reconstructTree(root, tree);
				}
				return tree;
			}


			// Print the tree to std::cout
			inline void printTree() const
			{
				printTree(root, 0);
			}


			// Print the code to std::cout
			inline void printCode() const
			{
				for (const auto& p : code) {
					std::cout << p.first << " -> ";
					for (int bit = p.second.second - 1 ; bit >= 0 ; --bit)
					std::cout << ((p.second.first >> bit) & 0x01 ? '1' : '0');
					std::cout << std::endl;
				}
			}


			// Get the representation of the tree used for storing it to file
			inline std::vector<bool> treeRepr() const
			{
				return treeRepr(root);
			}


			// Returns a codeword together with its length in bits
			inline std::pair<uint64_t, uint8_t> encode(WordT word) const
			{
				return code.find(word)->second;
			}


			// Returns the word together with an iterator to the next bit
			template<typename Iterator>
			inline std::pair<WordT, Iterator> decode(Iterator data) const
			{
				static_assert(std::is_same<typename Iterator::value_type, bool>::value, "Iterator must return bool");
				Node* current_node = root;
				while (!current_node->isLeaf()) {
					if (*data)
					current_node = current_node->right;
					else
					current_node = current_node->left;
					++data;
				}
				return std::make_pair(current_node->data, data);
			}



		private:
			static constexpr int bits_per_word = std::numeric_limits<std::make_unsigned_t<WordT>>::digits;
			Node* root = nullptr;
			// The code map maps a word to a pair containing the code word and its length
			std::unordered_map<WordT, std::pair<uint64_t, uint8_t>> code;


			template<typename Iterator>
			inline std::map<WordT, uint64_t> makeFrequencyTable(Iterator begin, Iterator end) const
			{
				static_assert(std::is_same<typename Iterator::value_type, WordT>::value,
						  "Iterator has wrong type");
				std::map<WordT, uint64_t> table;
				for (Iterator it = begin ; it != end ; ++it) {
					auto elem_iter = table.insert(std::make_pair(*it, 0)).first;
					elem_iter->second += 1;
				}
				return table;
			}


			// Left is encoded as a 0, right as a 1
			inline void makeCode(const Node* node, uint64_t path, uint8_t length)
			{
				if (node->isLeaf())
					code.insert(std::make_pair(node->data, std::make_pair(path, std::max(length, uint8_t(1)))));
				else {
					makeCode(node->left , (path << 1) | 0, length+1);
					makeCode(node->right, (path << 1) | 1, length+1);
				}
			}


			// Reconstructs a partial tree from the given data
			// Returns an iterator to the bit after the space used to store the tree
			template<typename Iterator>
			inline Iterator reconstructTree(Node* node, Iterator tree)
			{
				static_assert(std::is_same<typename Iterator::value_type, bool>::value, "Iterator must return bool");
				// Left tree
				if (*tree) {
					++tree;
					node->left = new Node(0, 0);
					for (int i = bits_per_word - 1 ; i >= 0 ; --i) {
					node->left->data |= static_cast<WordT>(*tree) << i;
					++tree;
					}
				}
				else {
					++tree;
					node->left = new Node(0, 0);
					tree = reconstructTree(node->left, tree);
				}
				// Right tree
				if (*tree) {
					++tree;
					node->right = new Node(0, 0);
					for (int i = bits_per_word - 1 ; i >= 0 ; --i) {
					node->right->data |= static_cast<WordT>(*tree) << i;
					++tree;
					}
				}
				else {
					++tree;
					node->right = new Node(0, 0);
					tree = reconstructTree(node->right, tree);
				}
				return tree;
			}


			inline void printTree(const Node* node, uint8_t depth) const
			{
				// Tabbing
				for (int i = 0 ; i < depth ; ++i)
					std::cout << "|  ";
				std::cout << "freq: " << node->frequency;
				if (node->isLeaf())
					std::cout << "  data: " << node->data << std::endl;
				else {
					std::cout << std::endl;
					printTree(node->left , depth+1);
					printTree(node->right, depth+1);
				}
			}


			inline std::vector<bool> treeRepr(const Node* node) const
			{
				std::vector<bool> res(1, node->isLeaf());
				if (node->isLeaf())
					for (int bit = bits_per_word-1 ; bit >= 0 ; --bit)
					res.push_back((node->data >> bit) & WordT(1));
				else {
					for (const auto& bit : treeRepr(node->left))
					res.push_back(bit);
					for (const auto& bit : treeRepr(node->right))
					res.push_back(bit);
				}
				return res;
			}


			inline void swap(HuffmanTree<WordT>& other)
			{
				std::swap(root, other.root);
				std::swap(code, other.code);
			}
		};
	}
	
	
	// Compresses the given range to a vector of booleans.
	// The compressed data consists of two parts.
	// The first part is the Huffman Tree used for encoding,
	// the second part is the actual compressed data.
	// The huffman tree is stored as a series of bits, where
	// a 0 indicates a non-leaf node and a 1 indicates a leaf node.
	// After every leaf node, the word it encodes is stored.
	// The branches get stored in the following order:
	//    this -> left -> right
	// The following example demonstrates this:
	//      .
	//     / \
	//    A   .     =>   0 1 01000001 0 1 01000010 1 01000011
	//       / \         . A          . B          C
	//      B   C
	// The second part of the compressed data are just a series
	// of codewords that are generated by looking at the location
	// of the word to compress in the huffman tree.
	// 0 stands for left and 1 stands for right.
	// In the example above, the codewords would be as follows:
	//    A -> 0
	//    B -> 10
	//    C -> 11
	template<typename Iterator>
	inline std::vector<bool> compress(Iterator begin, Iterator end)
	{
		// The word type is inferred from the iterator type
		using word_type = typename Iterator::value_type;
		// The word type has to be an integral value
		static_assert(std::numeric_limits<word_type>::is_integer, "The type to compress must be an integral type");
		// Build a huffman tree of the data
		auto huffman_tree = detail::HuffmanTree<word_type>::fromUncompressed(begin, end);
		// Store the compressed data inside a vector of booleans
		std::vector<bool> compressed(huffman_tree.treeRepr());  // Store the huffman tree itself
		for (Iterator it = begin ; it != end ; ++it) {
		auto code = huffman_tree.encode(*it);
		for (int bit = code.second - 1 ; bit >= 0 ; --bit)
			compressed.push_back((code.first >> bit) & 0x01);
		}
		return compressed;
	}


	// Decompresses the given data to a vector of WordTs
	template<typename WordT, typename Iterator>
	inline std::vector<WordT> decompress(Iterator begin, Iterator end)
	{
		static_assert(std::is_same<typename Iterator::value_type, bool>::value, "Iterator must return bool");
		// Reconstruct the huffman tree
		auto res = detail::HuffmanTree<WordT>::fromCompressed(begin);
		auto huffman_tree = res.first;
		begin = res.second;
		// Store the decompressed data inside a vector of WordTs
		std::vector<WordT> decompressed;
		while (begin != end) {
		auto res = huffman_tree.decode(begin);
		decompressed.push_back(res.first);
		begin = res.second;
		}
		return decompressed;
	}
}