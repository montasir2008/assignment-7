/*
 * File: huffman.cpp
 * --------------------------
 * Name: Montasir Mahir & Weijia Li
 * Course: CS106B
 * Section: Thu 7:30 pm
 * Description: This file implements Huffman compression and decompression
 * using encoding trees. It builds trees based on character frequencies,
 * encodes and decodes text, and flattens and reconstructs trees.
 */
#include "bits.h"
#include "treenode.h"
#include "huffman.h"
#include "map.h"
#include "vector.h"
#include "priorityqueue.h"
#include "strlib.h"
#include "SimpleTest.h"  // IWYU pragma: keep (needed to quiet spurious warning)
using namespace std;

/**
 * Given a Queue<Bit> containing the compressed message bits and the encoding tree
 * used to encode those bits, decode the bits back to the original message text.
 *
 * You can assume that tree is a well-formed non-empty encoding tree and
 * messageBits queue contains a valid sequence of encoded bits.
 *
 * Your implementation may change the messageBits queue however you like. There
 * are no requirements about what it should look like after this function
 * returns. The encoding tree should be unchanged.
 *
 * This function follows the bits through the encoding tree. Whenever
 * it reaches a leaf, it adds the character to the result and returns
 * to the root.
 */
string decodeText(EncodingTreeNode* tree, Queue<Bit>& messageBits) {
    EncodingTreeNode* current = tree;
    string result;
    while (!messageBits.isEmpty()) {
        if (messageBits.dequeue() == 0) {
            current = current->zero;
        } else {
            current = current->one;
        }

        if (current->isLeaf()) {
            result += current->getChar();
            current = tree;
        }
    }
    return result;
}

/**
 * Reconstruct encoding tree from flattened form Queue<Bit> and Queue<char>.
 *
 * You can assume that the queues are well-formed and represent
 * a valid encoding tree.
 *
 * Your implementation may change the queue parameters however you like. There
 * are no requirements about what they should look like after this function
 * returns.
 *
 * This function recursively reconstructs the encoding tree. A 0 creates
 * a leaf node, while a 1 creates an internal node with two subtrees.
 */
EncodingTreeNode* unflattenTree(Queue<Bit>& treeShape, Queue<char>& treeLeaves) {
    if (treeShape.isEmpty() && treeLeaves.isEmpty()) {
        return nullptr;
    }
    if (treeShape.dequeue() == 1) {
        EncodingTreeNode* z = unflattenTree(treeShape, treeLeaves);
        EncodingTreeNode* o = unflattenTree(treeShape, treeLeaves);
        EncodingTreeNode* root = new EncodingTreeNode(z,o);
        return root;
    }
    else {
        EncodingTreeNode* leaf = new EncodingTreeNode(treeLeaves.dequeue());
        return leaf;
    }
}

/**
 * Decompress the given EncodedData and return the original text.
 *
 * You can assume the input data is well-formed and was created by a correct
 * implementation of compress.
 *
 * Your implementation may change the data parameter however you like. There
 * are no requirements about what it should look like after this function
 * returns.
 *
 * This function reconstructs the encoding tree, decodes the message
 * bits, and deallocates the tree before returning the original text.
 */
string decompress(EncodedData& data) {
    EncodingTreeNode* root = unflattenTree(data.treeShape, data.treeLeaves);
    string result = decodeText(root, data.messageBits);

    deallocateTree(root);

    return result;
}

/**
 * Constructs an optimal Huffman coding tree for the given text, using
 * the algorithm described in lecture.
 *
 * Reports an error if the input text does not contain at least
 * two distinct characters.
 *
 * When assembling larger trees out of smaller ones, make sure to set the first
 * tree dequeued from the queue to be the zero subtree of the new tree and the
 * second tree as the one subtree.
 *
 * This function counts character frequencies and creates a leaf for
 * each character. It uses a priority queue to repeatedly combine the
 * two trees with the lowest priorities until one tree remains.
 */
EncodingTreeNode* buildHuffmanTree(string text) {
    Map<char, int> frequencies;
    for (char c : text) {
        frequencies[c]++;
    }
    if (frequencies.size() < 2) {
        error("String must contain at least two distinct chars");
    }

    PriorityQueue<EncodingTreeNode*> tree;
    for (char c : frequencies) {
        EncodingTreeNode* leaf = new EncodingTreeNode(c);
        tree.enqueue(leaf, frequencies[c]);
    }

    while (tree.size() != 1) {
        int totalp = 0;
        totalp += tree.peekPriority();
        EncodingTreeNode* leaf1 = tree.dequeue();
        totalp += tree.peekPriority();
        EncodingTreeNode* leaf2 = tree.dequeue();

        EncodingTreeNode* interior = new EncodingTreeNode(leaf1, leaf2);
        tree.enqueue(interior, totalp);
    }

    return tree.dequeue();
}

/*
 * This helper recursively traverses the tree and maps each character
 * to its bit sequence. Each recursive branch gets its own copy of
 * the current path.
 */
void encodeMap(EncodingTreeNode* tree, Queue<Bit> path, Map<char, Queue<Bit>>& charMap) {
    if (tree == nullptr) {
        return;
    }
    if (tree->isLeaf()) {
        charMap.put(tree->getChar(), path);
        return;
    }
    else {
        Queue<Bit> zeroPath = path;
        zeroPath.enqueue(0);
        encodeMap(tree->zero, zeroPath, charMap);

        Queue<Bit> onePath = path;
        onePath.enqueue(1);
        encodeMap(tree->one, onePath, charMap);
    }
}

/**
 * Given a string and an encoding tree, encode the text using the tree
 * and return a Queue<Bit> of the encoded bit sequence.
 *
 * You can assume tree is a valid non-empty encoding tree and contains an
 * encoding for every character in the text.
 *
 * This function builds a map of character encodings and iterates
 * through the text, adding each character's bits to the result queue.
 */
Queue<Bit> encodeText(EncodingTreeNode* tree, string text) {
    Queue<Bit> result;
    Queue<Bit> path;
    Map<char, Queue<Bit>> charMap;
    encodeMap(tree, path, charMap);

    for (char c : text) {
        Queue<Bit> temp = charMap[c];
        while (!temp.isEmpty()) {
            result.enqueue(temp.dequeue());
        }
    }

    return result;
}

/**
 * Flatten the given tree into a Queue<Bit> and Queue<char> in the manner
 * specified in the assignment writeup.
 *
 * You can assume the input queues are empty on entry to this function.
 *
 * You can assume tree is a valid well-formed encoding tree.
 *
 * This function recursively flattens the tree by recording a 1 for
 * internal nodes and a 0 for leaves. Leaf characters are added to
 * the treeLeaves queue from left to right.
 */
void flattenTree(EncodingTreeNode* tree, Queue<Bit>& treeShape, Queue<char>& treeLeaves) {
    if (tree->isLeaf()) {
        treeLeaves.enqueue(tree->getChar());
        treeShape.enqueue(0);
        return;
    }
    treeShape.enqueue(1);
    flattenTree(tree->zero, treeShape, treeLeaves);
    flattenTree(tree->one, treeShape, treeLeaves);
}

/**
 * Compress the input text using Huffman coding, producing as output
 * an EncodedData containing the encoded message and flattened
 * encoding tree used.
 *
 * Reports an error if the message text does not contain at least
 * two distinct characters.
 *
 * This function builds a Huffman tree, encodes the message, and
 * flattens the tree. It stores the results in EncodedData and
 * deallocates the tree before returning.
 */
EncodedData compress(string messageText) {
    EncodedData data;
    EncodingTreeNode* tree = buildHuffmanTree(messageText);
    Queue<Bit> message = encodeText(tree, messageText);

    Queue<Bit> treeShape;
    Queue<char> treeLeaves;
    flattenTree(tree, treeShape, treeLeaves);
    data.messageBits = message;
    data.treeLeaves = treeLeaves;
    data.treeShape = treeShape;

    deallocateTree(tree);

    return data;
}

/* * * * * * Testing Helper Functions Below This Point * * * * * */

EncodingTreeNode* createExampleTree() {
    EncodingTreeNode* t = new EncodingTreeNode('T');
    EncodingTreeNode* e = new EncodingTreeNode('E');
    EncodingTreeNode* r = new EncodingTreeNode('R');
    EncodingTreeNode* s = new EncodingTreeNode('S');
    EncodingTreeNode* int1 = new EncodingTreeNode(r,s);
    EncodingTreeNode* int2 = new EncodingTreeNode(int1,e);
    EncodingTreeNode* root = new EncodingTreeNode(t,int2);
    return root;
}

void deallocateTree(EncodingTreeNode* t) {
    if (t == nullptr) {
        return;
    }
    deallocateTree(t->zero);
    deallocateTree(t->one);
    delete t;
}

bool areEqual(EncodingTreeNode* a, EncodingTreeNode* b) {
    if (a == nullptr && b == nullptr) {
        return true;
    }
    else if (a == nullptr || b == nullptr) {
        return false;
    }
    else if (a->isLeaf() && b->isLeaf()) {
        if (a->getChar() == b->getChar()) return true;
    }
    else if (!a->isLeaf() && !b->isLeaf()) {
        bool sub1 = areEqual(a->zero, b->zero);
        bool sub2 = areEqual(a->one, b->one);
        return sub1 && sub2;
    }
    return false;
}

/* * * * * * Test Cases Below This Point * * * * * */

/* TODO: Write your own student tests. */

STUDENT_TEST("Test deallocateTree on example tree") {
    EncodingTreeNode* root = createExampleTree();
    deallocateTree(root);
}

STUDENT_TEST("Two empty trees") {
    EXPECT(areEqual(nullptr, nullptr));
}

STUDENT_TEST("Empty tree and nonempty tree") {
    EncodingTreeNode* a = new EncodingTreeNode(
        new EncodingTreeNode('A'), new EncodingTreeNode('B'));

    EXPECT(!areEqual(a, nullptr));
    EXPECT(!areEqual(nullptr, a));

    deallocateTree(a);
}

STUDENT_TEST("Two identical simple trees") {
    EncodingTreeNode* a = new EncodingTreeNode(
        new EncodingTreeNode('A'), new EncodingTreeNode('B'));
    EncodingTreeNode* b = new EncodingTreeNode(
        new EncodingTreeNode('A'), new EncodingTreeNode('B'));

    EXPECT(areEqual(a, b));

    deallocateTree(a);
    deallocateTree(b);
}

STUDENT_TEST("Trees with swapped children") {
    EncodingTreeNode* a = new EncodingTreeNode(
        new EncodingTreeNode('A'), new EncodingTreeNode('B'));
    EncodingTreeNode* b = new EncodingTreeNode(
        new EncodingTreeNode('B'), new EncodingTreeNode('A'));

    EXPECT(!areEqual(a, b));

    deallocateTree(a);
    deallocateTree(b);
}

STUDENT_TEST("Trees with different characters") {
    EncodingTreeNode* a = new EncodingTreeNode('A');
    EncodingTreeNode* b = new EncodingTreeNode('B');

    EXPECT(!areEqual(a, b));

    deallocateTree(a);
    deallocateTree(b);
}

STUDENT_TEST("Example tree and simple tree") {
    EncodingTreeNode* a = createExampleTree();
    EncodingTreeNode* b = new EncodingTreeNode(
        new EncodingTreeNode('A'), new EncodingTreeNode('B'));

    EXPECT(!areEqual(a, b));

    deallocateTree(a);
    deallocateTree(b);
}

STUDENT_TEST("Two identical example trees") {
    EncodingTreeNode* a = createExampleTree();
    EncodingTreeNode* b = createExampleTree();

    EXPECT(areEqual(a, b));

    deallocateTree(a);
    deallocateTree(b);
}

STUDENT_TEST("Example tree compared to its subtree") {
    EncodingTreeNode* a = createExampleTree();

    EXPECT(!areEqual(a, a->zero));
    EXPECT(!areEqual(a, a->one));

    deallocateTree(a);
}

STUDENT_TEST("decodeText with empty message") {
    EncodingTreeNode* tree = new EncodingTreeNode(
        new EncodingTreeNode('A'),
        new EncodingTreeNode('B')
        );

    Queue<Bit> bits = {};

    EXPECT_EQUAL(decodeText(tree, bits), "");

    deallocateTree(tree);
}

STUDENT_TEST("decodeText with one character") {
    EncodingTreeNode* tree = new EncodingTreeNode(
        new EncodingTreeNode('A'),
        new EncodingTreeNode('B')
        );

    Queue<Bit> bits = {0};

    EXPECT_EQUAL(decodeText(tree, bits), "A");
    EXPECT(bits.isEmpty());

    deallocateTree(tree);
}

STUDENT_TEST("decodeText with repeated characters") {
    EncodingTreeNode* tree = new EncodingTreeNode(
        new EncodingTreeNode('A'),
        new EncodingTreeNode('B')
        );

    Queue<Bit> bits = {0, 0, 0, 0};

    EXPECT_EQUAL(decodeText(tree, bits), "AAAA");
    EXPECT(bits.isEmpty());

    deallocateTree(tree);
}

STUDENT_TEST("decodeText with variable length encodings") {
    EncodingTreeNode* tree = new EncodingTreeNode(
        new EncodingTreeNode('A'),
        new EncodingTreeNode(
            new EncodingTreeNode('B'),
            new EncodingTreeNode('C')
            )
        );

    Queue<Bit> bits = {0, 1, 0, 1, 1, 0};

    EXPECT_EQUAL(decodeText(tree, bits), "ABCA");
    EXPECT(bits.isEmpty());

    deallocateTree(tree);
}

STUDENT_TEST("decompress warmup question 1") {
    EncodedData data;
    data.treeShape = {1, 1, 0, 1, 0, 0, 0};
    data.treeLeaves = {'N', 'M', 'S', 'O'};
    data.messageBits = {0, 1, 0, 1, 1, 0, 0, 0, 1, 1};

    EXPECT_EQUAL(decompress(data), "MOONS");
}

STUDENT_TEST("decompress warmup question 2 - SONS") {
    EncodedData data;
    data.treeShape = {1, 1, 0, 1, 0, 0, 0};
    data.treeLeaves = {'N', 'M', 'S', 'O'};
    data.messageBits = {0, 1, 1, 1, 0, 0, 0, 1, 1};

    EXPECT_EQUAL(decompress(data), "SONS");
}

STUDENT_TEST("decompress with simple two-leaf tree") {
    EncodedData data;
    data.treeShape = {1, 0, 0};
    data.treeLeaves = {'A', 'B'};
    data.messageBits = {0, 1, 1, 0};

    EXPECT_EQUAL(decompress(data), "ABBA");
}

STUDENT_TEST("decompress with repeated characters") {
    EncodedData data;
    data.treeShape = {1, 0, 1, 0, 0};
    data.treeLeaves = {'A', 'B', 'C'};
    data.messageBits = {0, 0, 0, 1, 0, 1, 0, 1, 1};

    EXPECT_EQUAL(decompress(data), "AAABBC");
}

STUDENT_TEST("encodeText with empty string") {
    EncodingTreeNode* tree = createExampleTree();

    EXPECT(encodeText(tree, "").isEmpty());

    deallocateTree(tree);
}

STUDENT_TEST("encodeText with variable length encodings") {
    EncodingTreeNode* tree = createExampleTree();

    Queue<Bit> expected = {0, 1, 0, 0, 1, 0, 1, 1, 1};
    EXPECT_EQUAL(encodeText(tree, "TRSE"), expected);

    deallocateTree(tree);
}

STUDENT_TEST("encodeText and decodeText round-trip") {
    EncodingTreeNode* tree = createExampleTree();

    string original = "STREETS";
    Queue<Bit> encoded = encodeText(tree, original);

    EXPECT_EQUAL(decodeText(tree, encoded), original);

    deallocateTree(tree);
}

STUDENT_TEST("flattenTree with simple two-leaf tree") {
    EncodingTreeNode* tree = new EncodingTreeNode(
        new EncodingTreeNode('A'),
        new EncodingTreeNode('B')
        );

    Queue<Bit> treeShape;
    Queue<char> treeLeaves;

    flattenTree(tree, treeShape, treeLeaves);

    Queue<Bit> expectedShape = {1, 0, 0};
    Queue<char> expectedLeaves = {'A', 'B'};

    EXPECT_EQUAL(treeShape, expectedShape);
    EXPECT_EQUAL(treeLeaves, expectedLeaves);

    deallocateTree(tree);
}

STUDENT_TEST("flattenTree and unflattenTree round-trip") {
    EncodingTreeNode* original = createExampleTree();

    Queue<Bit> treeShape;
    Queue<char> treeLeaves;

    flattenTree(original, treeShape, treeLeaves);
    EncodingTreeNode* reconstructed = unflattenTree(treeShape, treeLeaves);

    EXPECT(areEqual(original, reconstructed));

    deallocateTree(original);
    deallocateTree(reconstructed);
}

STUDENT_TEST("flattenTree and unflattenTree with left-heavy tree") {
    EncodingTreeNode* original = new EncodingTreeNode(
        new EncodingTreeNode(
            new EncodingTreeNode('A'),
            new EncodingTreeNode('B')
            ),
        new EncodingTreeNode('C')
        );

    Queue<Bit> treeShape;
    Queue<char> treeLeaves;

    flattenTree(original, treeShape, treeLeaves);
    EncodingTreeNode* reconstructed = unflattenTree(treeShape, treeLeaves);

    EXPECT(areEqual(original, reconstructed));

    deallocateTree(original);
    deallocateTree(reconstructed);
}

STUDENT_TEST("buildHuffmanTree with invalid inputs") {
    EXPECT_ERROR(buildHuffmanTree(""));
    EXPECT_ERROR(buildHuffmanTree("A"));
    EXPECT_ERROR(buildHuffmanTree("AAAAAA"));
}

STUDENT_TEST("buildHuffmanTree with distinct frequencies") {
    EncodingTreeNode* tree = buildHuffmanTree("AAAAAABBBCC");

    EncodingTreeNode* expected = new EncodingTreeNode(
        new EncodingTreeNode(
            new EncodingTreeNode('C'),
            new EncodingTreeNode('B')
            ),
        new EncodingTreeNode('A')
        );

    EXPECT(areEqual(tree, expected));

    deallocateTree(tree);
    deallocateTree(expected);
}

STUDENT_TEST("buildHuffmanTree with tied frequencies") {
    string text = "AABBCCDD";
    EncodingTreeNode* tree = buildHuffmanTree(text);

    Queue<Bit> encoded = encodeText(tree, text);

    EXPECT_EQUAL(encoded.size(), 16);
    EXPECT_EQUAL(decodeText(tree, encoded), text);

    deallocateTree(tree);
}

STUDENT_TEST("compress and decompress with repeated characters") {
    string original = "AAAAAABBBCC";

    EncodedData data = compress(original);

    EXPECT_EQUAL(decompress(data), original);
}

STUDENT_TEST("compress and decompress with spaces and punctuation") {
    string original = "Hello, World! Huffman coding is fun.";

    EncodedData data = compress(original);

    EXPECT_EQUAL(decompress(data), original);
}

/* * * * * Provided Tests Below This Point * * * * */

PROVIDED_TEST("decodeText, small example encoding tree") {
    EncodingTreeNode* tree = createExampleTree(); // see diagram above
    EXPECT(tree != nullptr);

    Queue<Bit> messageBits = { 1, 1 }; // E
    EXPECT_EQUAL(decodeText(tree, messageBits), "E");

    messageBits = { 1, 0, 1, 1, 1, 0 }; // SET
    EXPECT_EQUAL(decodeText(tree, messageBits), "SET");

    messageBits = { 1, 0, 1, 0, 1, 0, 0, 1, 1, 1, 1, 0, 1, 0, 1}; // STREETS
    EXPECT_EQUAL(decodeText(tree, messageBits), "STREETS");

    deallocateTree(tree);
}

PROVIDED_TEST("unflattenTree, small example encoding tree") {
    EncodingTreeNode* reference = createExampleTree(); // see diagram above
    Queue<Bit>  treeShape  = { 1, 0, 1, 1, 0, 0, 0 };
    Queue<char> treeLeaves = { 'T', 'R', 'S', 'E' };
    EncodingTreeNode* tree = unflattenTree(treeShape, treeLeaves);

    EXPECT(areEqual(tree, reference));

    deallocateTree(tree);
    deallocateTree(reference);
}

PROVIDED_TEST("decompress, small example input") {
    EncodedData data = {
        { 1, 0, 1, 1, 0, 0, 0 }, // treeShape
        { 'T', 'R', 'S', 'E' },  // treeLeaves
        { 0, 1, 0, 0, 1, 1, 1, 0, 1, 1, 0, 1 } // messageBits
    };

    EXPECT_EQUAL(decompress(data), "TRESS");
}

PROVIDED_TEST("buildHuffmanTree, small example encoding tree") {
    EncodingTreeNode* reference = createExampleTree(); // see diagram above
    EncodingTreeNode* tree = buildHuffmanTree("STREETTEST");
    EXPECT(areEqual(tree, reference));

    deallocateTree(reference);
    deallocateTree(tree);
}

PROVIDED_TEST("encodeText, small example encoding tree") {
    EncodingTreeNode* reference = createExampleTree(); // see diagram above

    Queue<Bit> messageBits = { 1, 1 }; // E
    EXPECT_EQUAL(encodeText(reference, "E"), messageBits);

    messageBits = { 1, 0, 1, 1, 1, 0 }; // SET
    EXPECT_EQUAL(encodeText(reference, "SET"), messageBits);

    messageBits = { 1, 0, 1, 0, 1, 0, 0, 1, 1, 1, 1, 0, 1, 0, 1 }; // STREETS
    EXPECT_EQUAL(encodeText(reference, "STREETS"), messageBits);

    deallocateTree(reference);
}

PROVIDED_TEST("flattenTree, small example encoding tree") {
    EncodingTreeNode* reference = createExampleTree(); // see diagram above
    Queue<Bit>  expectedShape  = { 1, 0, 1, 1, 0, 0, 0 };
    Queue<char> expectedLeaves = { 'T', 'R', 'S', 'E' };

    Queue<Bit>  treeShape;
    Queue<char> treeLeaves;
    flattenTree(reference, treeShape, treeLeaves);

    EXPECT_EQUAL(treeShape,  expectedShape);
    EXPECT_EQUAL(treeLeaves, expectedLeaves);

    deallocateTree(reference);
}

PROVIDED_TEST("compress, small example input") {
    EncodedData data = compress("STREETTEST");
    Queue<Bit>  treeShape   = { 1, 0, 1, 1, 0, 0, 0 };
    Queue<char> treeChars   = { 'T', 'R', 'S', 'E' };
    Queue<Bit>  messageBits = { 1, 0, 1, 0, 1, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 0, 1, 0 };

    EXPECT_EQUAL(data.treeShape, treeShape);
    EXPECT_EQUAL(data.treeLeaves, treeChars);
    EXPECT_EQUAL(data.messageBits, messageBits);
}

PROVIDED_TEST("Test end-to-end compress -> decompress") {
    Vector<string> inputs = {
        "HAPPY HIP HOP",
        "Nana Nana Nana Nana Nana Nana Nana Nana Batman",
        "Research is formalized curiosity. It is poking and prying with a purpose. – Zora Neale Hurston",
    };

    for (string input: inputs) {
        EncodedData data = compress(input);
        string output = decompress(data);

        EXPECT_EQUAL(input, output);
    }
}
