import heapq

class HuffmanNode:
    def __init__(self, ch=None, freq=0):
        self.ch = ch
        self.freq = freq
        self.left = None
        self.right = None
    
    # For heapq
    def __lt__(self, other):
        # Tie-breaking: smaller char first if freq equal
        if self.freq != other.freq:
            return self.freq < other.freq
        if self.ch is None: return False
        if other.ch is None: return True
        return self.ch < other.ch

    @staticmethod
    def build_huffman_tree(freq_table: dict):
        heap = []
        for ch, freq in freq_table.items():
            heapq.heappush(heap, HuffmanNode(ch, freq))

        while len(heap) > 1:
            a = heapq.heappop(heap)
            b = heapq.heappop(heap)

            parent = HuffmanNode('$', a.freq + b.freq)
            parent.left = a
            parent.right = b
            heapq.heappush(heap, parent)

        return heap[0]

    @staticmethod
    def decode_huffman(encoded_data: str, root):
        decoded = ""
        node = root
        for bit in encoded_data:
            node = node.left if bit == '0' else node.right
            if node.left is None and node.right is None:
                decoded += node.ch
                node = root
        return decoded

    @staticmethod
    def decode(encoded_data: str, freq_table: dict):
        root = HuffmanNode.build_huffman_tree(freq_table)
        return HuffmanNode.decode_huffman(encoded_data, root)
