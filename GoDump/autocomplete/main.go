package main

import "fmt"

func main() {
	trie := NewTrie()
	list := []string{
		"And", "Award", "Ant", "Ambrose", "Apostle", "Anteater", "An",
	}

	for _, item := range list {
		trie.Insert(item)
	}

	fmt.Println(trie.Search("A"))
	trie.Delete("Ant")
	fmt.Println(trie.Search("A"))
}
