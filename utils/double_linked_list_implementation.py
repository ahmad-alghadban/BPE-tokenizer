class Node:
    __slots__ = ("data", "prev", "next")

    def __init__(self, data):
        self.data = data
        self.prev = None
        self.next = None


class LinkedList:
    __slots__ = ("head", "tail", "sz")

    def __init__(self):
        self.head = None
        self.tail = None
        self.sz = 0

    def __len__(self):
        return self.sz

    def __iter__(self):
        cur = self.head
        while cur:
            yield cur.data
            cur = cur.next

    def __repr__(self):
        return f"LinkedList({list(self)!r})"

    def empty(self):
        return self.sz == 0

    def _raise_if_empty(self):
        if not self.head:
            raise IndexError("List is empty")

    # --- Modifiers ---
    def push_back(self, value):
        node = Node(value)
        if not self.head:
            self.head = self.tail = node
        else:
            self.tail.next = node
            node.prev = self.tail
            self.tail = node
        self.sz += 1
        return node

    def push_front(self, value):
        node = Node(value)
        if not self.head:
            self.head = self.tail = node
        else:
            node.next = self.head
            self.head.prev = node
            self.head = node
        self.sz += 1
        return node

    def pop_back(self):
        self._raise_if_empty()
        node = self.tail
        self.tail = node.prev
        if self.tail:
            self.tail.next = None
        else:
            self.head = None
        self.sz -= 1
        return node

    def pop_front(self):
        self._raise_if_empty()
        node = self.head
        self.head = node.next
        if self.head:
            self.head.prev = None
        else:
            self.tail = None
        self.sz -= 1
        return node

    # --- Insert ---
    def insert(self, node, value):
        """
        Insert before `node` (which must be a Node).
        If node is None -> insert at end.
        """
        if node is None:
            return self.push_back(value)

        new_node = Node(value)
        new_node.next = node
        new_node.prev = node.prev

        if node.prev:
            node.prev.next = new_node
        else:
            self.head = new_node

        node.prev = new_node
        self.sz += 1
        return new_node

    # --- Erase ---
    def erase(self, node):
        if node is None:
            raise IndexError("Invalid node")

        if node.prev:
            node.prev.next = node.next
        else:
            self.head = node.next

        if node.next:
            node.next.prev = node.prev
        else:
            self.tail = node.prev

        # Detach the removed node so any stale reference fails fast
        # instead of silently traversing back into the list.
        node.prev = node.next = None

        self.sz -= 1
        return node

    # --- Access ---
    def front(self):
        self._raise_if_empty()
        return self.head.data

    def back(self):
        self._raise_if_empty()
        return self.tail.data

    def back_iter(self):
        self._raise_if_empty()
        return self.tail

    def begin(self):
        self._raise_if_empty()
        return self.head

    # --- Export ---
    def export_as_list(self):
        return list(self)
