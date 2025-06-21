package datastruct

type TrieNode struct {
	word bool
	char rune
	next map[rune]*TrieNode
}

func NewTrieNode(c rune) *TrieNode {
	trieNode := new(TrieNode)
	trieNode.char = c
	trieNode.next = make(map[rune]*TrieNode)
	return trieNode
}

func (trieNode *TrieNode) List(s string, list *[]string) {
	s = s + string(trieNode.char)
	if trieNode.word {
		*list = append(*list, s)
	}
	for _, next := range trieNode.next {
		next.List(s, list)
	}
}

func (trieNode *TrieNode) IsEmpty() bool {
	return len(trieNode.next) == 0
}

type Trie struct {
	Root *TrieNode
}

func NewTrie() *Trie {
	trie := new(Trie)
	trie.Root = NewTrieNode(' ')
	return trie
}

func (trie *Trie) Insert(s string) {
	node := trie.Root

	for _, c := range s {
		if next, exists := node.next[c]; exists {
			node = next
			continue
		}
		node.next[c] = NewTrieNode(c)
		node = node.next[c]
	}
	if node != trie.Root {
		node.word = true
	}
}

func (trie *Trie) IsEmpty() bool {
	return trie.Root.IsEmpty()
}

func (trie *Trie) Delete(s string) {
	trieDelHelper(trie.Root, s, 0)
}

func trieDelHelper(trieNode *TrieNode, s string, depth int) *TrieNode {
	if len(s) == 0 {
		trieNode.word = false
		if trieNode.IsEmpty() {
			trieNode = nil
		}
		return trieNode
	}

	if next, exists := trieNode.next[rune(s[0])]; exists {
		next = trieDelHelper(next, s[1:], depth+1)
		if next == nil {
			delete(trieNode.next, rune(s[0]))
		}
		if trieNode.IsEmpty() && !trieNode.word {
			trieNode = nil
		}
	}
	return trieNode
}

func (trie *Trie) Search(s string) []string {
	node := trie.Root
	sug := []string{}

	for _, c := range s {
		if next, exists := node.next[c]; exists {
			node = next
			continue
		} else {
			return sug
		}
	}
	node.List(s[:len(s)-1], &sug)
	return sug
}

func (trie *Trie) List() []string {
	list := []string{}
	for _, next := range trie.Root.next {
		next.List("", &list)
	}
	return list
}
