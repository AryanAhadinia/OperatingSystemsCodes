struct Element
{
	const char *value;
	struct Element *prev;
	struct Element *next;
};
typedef struct Element LinkedList_t;
typedef struct Element Element_t;

void insert(LinkedList_t *list, Element_t *element);

int delete(Element_t *element);

Element_t *lookup(LinkedList_t *list, const char *value);

int get_length(LinkedList_t *list);
