package datastruct

type HeapNode[T any] struct {
	data     T
	priority uint
}

type HeapType int8

const (
	MaxHeap HeapType = 0
	MinHeap HeapType = 1
)

type Heap[T any] struct {
	data     []*HeapNode[T]
	heapType HeapType
}

func (heap *Heap[T]) Size() int {
	return len(heap.data)
}

func (heap *Heap[T]) Insert(item T, priority uint) {
	node := new(HeapNode[T])
	node.data = item
	node.priority = priority
	heap.data = append(heap.data, node)
	heap.InsertNode(node)
}

func (heap *Heap[T]) InsertNode(node *HeapNode[T]) {
	index := heap.Size() - 1
	switch heap.heapType {
	case MinHeap:
		for index > 0 && heap.data[(index-1)/2].priority > heap.data[index].priority {
			heap.data[index], heap.data[(index-1)/2] = heap.data[(index-1)/2], heap.data[index]
			index = (index - 1) / 2
		}
	case MaxHeap:
		for index > 0 && heap.data[(index-1)/2].priority < heap.data[index].priority {
			heap.data[index], heap.data[(index-1)/2] = heap.data[(index-1)/2], heap.data[index]
			index = (index - 1) / 2
		}
	}
}

func (heap *Heap[T]) Heapify(heapType HeapType) {
	lastNonLeaf := heap.Size()/2 - 1
	if lastNonLeaf < 0 {
		return
	}

	switch heapType {
	case MaxHeap:
		for i := lastNonLeaf; i >= 0; i-- {
			maxHeapify(heap, i)
		}
	case MinHeap:
		for i := lastNonLeaf; i >= 0; i-- {
			minHeapify(heap, i)
		}
	}
}

func maxHeapify[T any](heap *Heap[T], i int) {
	largest := i
	left := 2*i + 1
	right := 2*i + 2
	data := heap.data

	if left < len(data) && data[left].priority > data[largest].priority {
		largest = left
	}

	if right < len(data) && data[right].priority > data[largest].priority {
		largest = right
	}

	if largest != i {
		heap.data[i], heap.data[largest] = heap.data[largest], heap.data[i]
		maxHeapify(heap, largest)
	}
}

func minHeapify[T any](heap *Heap[T], i int) {
	smallest := i
	left := 2*i + 1
	right := 2*i + 2
	data := heap.data

	if left < len(data) && data[left].priority < data[smallest].priority {
		smallest = left
	}

	if right < len(data) && data[right].priority < data[smallest].priority {
		smallest = right
	}

	if smallest != i {
		heap.data[i], heap.data[smallest] = heap.data[smallest], heap.data[i]
		minHeapify(heap, smallest)
	}
}
