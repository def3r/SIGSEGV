package main

import (
	"fmt"
	"slices"
)

type TrieNode struct {
	word     bool
	char     rune
	parent   *TrieNode
	children map[rune]*TrieNode
}

func NewTrieNode(c rune) *TrieNode {
	trieNode := new(TrieNode)
	trieNode.char = c
	trieNode.parent = nil
	trieNode.children = make(map[rune]*TrieNode)
	return trieNode
}

func (trieNode *TrieNode) List(list *[]*TrieNode) {
	if trieNode.word {
		*list = append(*list, trieNode)
	}
	for _, child := range trieNode.children {
		child.List(list)
	}
}

func (trieNode *TrieNode) IsEmpty() bool {
	return len(trieNode.children) == 0
}

func (trieNode *TrieNode) InformChild(r rune) {
	trieNode.children[r].parent = trieNode
}

func (trieNode *TrieNode) GetString() string {
	if !trieNode.word {
		return "[Error]: TrieNode is not a word"
	}
	var s string
	for trieNode.parent != nil {
		s = string(trieNode.char) + s
		trieNode = trieNode.parent
	}
	return s
}

type Trie struct {
	Root  *TrieNode
	PList []*TrieNode
}

func (trie *Trie) Size() int {
	return len(trie.PList)
}

func (trie *Trie) At(index int) (string, error) {
	if index < 0 {
		return "", fmt.Errorf("Invalid index: %d", index)
	} else if index >= len(trie.PList) {
		return "", fmt.Errorf("Cannot access list index %d of size %d", index, len(trie.PList))
	}

	return trie.PList[index].GetString(), nil
}

func NewTrie() *Trie {
	trie := new(Trie)
	trie.Root = NewTrieNode(' ')
	return trie
}

func (trie *Trie) Insert(s string) {
	node := trie.Root

	for _, c := range s {
		if child, exists := node.children[c]; exists {
			node = child
			continue
		}
		node.children[c] = NewTrieNode(c)
		node.InformChild(c)
		node = node.children[c]
	}
	if node != trie.Root {
		node.word = true
	}
	trie.PList = append(trie.PList, node)
}

func (trie *Trie) IsEmpty() bool {
	return trie.Root.IsEmpty()
}

func (trie *Trie) Delete(s string) {
	trieDelHelper(trie, trie.Root, s, 0)
}

func trieDelHelper(trie *Trie, trieNode *TrieNode, s string, depth int) *TrieNode {
	if len(s) == 0 {
		trieNode.word = false
		trie.PList = slices.DeleteFunc(trie.PList, func(item *TrieNode) bool {
			return item == trieNode
		})
		if trieNode.IsEmpty() {
			trieNode = nil
		}
		return trieNode
	}

	if child, exists := trieNode.children[rune(s[0])]; exists {
		child = trieDelHelper(trie, child, s[1:], depth+1)
		if child == nil {
			delete(trieNode.children, rune(s[0]))
		}
		if trieNode.IsEmpty() && !trieNode.word {
			trieNode = nil
		}
	}
	return trieNode
}

func (trie *Trie) Search(s string) *Heap[*TrieNode] {
	node := trie.Root
	nodes := []*TrieNode{}
	pq := new(Heap[*TrieNode])

	for _, c := range s {
		if child, exists := node.children[c]; exists {
			node = child
			continue
		} else {
			return pq
		}
	}

	node.List(&nodes)
	for priority, ptr := range trie.PList {
		if slices.Index(nodes, ptr) != -1 {
			pq.Insert(ptr, uint(priority))
		}
	}
	return pq
}

func (trie *Trie) List() *[]*TrieNode {
	nodes := new([]*TrieNode)
	for _, child := range trie.Root.children {
		child.List(nodes)
	}
	return nodes
}
