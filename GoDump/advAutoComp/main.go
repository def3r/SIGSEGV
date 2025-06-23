package main

import (
	"bufio"
	"fmt"
	"os"
)

func main() {
	trie := NewTrie()
	fp, err := os.OpenFile(".dlshrc", os.O_RDONLY, 0644)
	if err != nil {
		panic(err)
	}
	defer fp.Close()

	scanner := bufio.NewScanner(fp)
	for scanner.Scan() {
		trie.Insert(scanner.Text())
	}

	trie.Delete("exit")

	heap := trie.Search("g")
	for heap.HasNext() {
		top, _ := heap.Next()
		fmt.Println(top.GetString())
	}
	top, err := heap.Top()
	fmt.Println(top.GetString())

	fmt.Println("----------")

	for heap.HasPrev() {
		top, _ := heap.Prev()
		fmt.Println(top.GetString())
	}
	top, err = heap.Top()
	fmt.Println(top.GetString())

	fmt.Println(trie.At(5))
}
