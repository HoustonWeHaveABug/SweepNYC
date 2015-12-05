#include <stdio.h>
#include <stdlib.h>

#define N_EDGE_TYPES 4UL

typedef struct node_s node_t;

struct path_s {
	node_t *to;
	int visited;
};
typedef struct path_s path_t;

struct node_s {
	unsigned long street;
	unsigned long avenue;
	unsigned long n_paths;
	path_t *paths;
	path_t *path_out;
	int polarity;
	int visited;
	node_t *from;
};

struct call_s {
	int type;
	node_t *node;
	path_t *path;
};
typedef struct call_s call_t;

int read_street(unsigned long);
int read_node(unsigned long, unsigned long);
path_t *add_path(node_t *, node_t *);
void set_path(path_t *, node_t *);
int read_edges(void);
int read_edge(const int *, unsigned long);
int read_separator(void);
node_t *positive_node(void);
int negative_polarity(node_t *);
void add_q_nodes(node_t *);
void search_circuit(node_t *);
void add_call(int, node_t *, path_t *);
void print_node(node_t *);
void free_data(void);

int *edges = NULL, *current_edge;
unsigned long n_streets, n_avenues, n_nodes, n_paths, n_calls;
node_t *nodes = NULL, *node_out, *current_node, **q_nodes = NULL, **q_node_out, **q_node_top;
call_t *calls, *call_top;

int main(void) {
unsigned long start_street, start_avenue, n_edges, street;
node_t *node, *positive, **q_node;
	if (scanf("%lu", &n_streets) != 1 || n_streets < 1) {
		fprintf(stderr, "Invalid number of streets\n");
		return EXIT_FAILURE;
	}
	if (scanf("%lu", &n_avenues) != 1 || n_avenues < 1) {
		fprintf(stderr, "Invalid number of avenues\n");
		return EXIT_FAILURE;
	}
	if (scanf("%lu", &start_street) != 1 || start_street < 1 || start_street > n_streets) {
		fprintf(stderr, "Invalid starting street\n");
		return EXIT_FAILURE;
	}
	if (scanf("%lu", &start_avenue) != 1 || start_avenue < 1 || start_avenue > n_avenues) {
		fprintf(stderr, "Invalid starting avenue\n");
		return EXIT_FAILURE;
	}
	fgetc(stdin);
	n_edges = n_streets*(n_streets-1)+n_avenues*(n_avenues-1);
	edges = malloc(sizeof(int)*n_edges);
	if (!edges) {
		fprintf(stderr, "Cannot allocate memory for edges\n");
		return EXIT_FAILURE;
	}
	n_nodes = n_streets*n_avenues;
	nodes = malloc(sizeof(node_t)*n_nodes);
	if (!nodes) {
		fprintf(stderr, "Cannot allocate memory for nodes\n");
		free_data();
		return EXIT_FAILURE;
	}
	node_out = nodes+n_nodes;
	n_paths = 0;
	current_edge = edges;
	current_node = nodes;
	if (!read_street(1UL)) {
		free_data();
		return EXIT_FAILURE;
	}
	for (street = 2; street <= n_streets; street++) {
		if (!read_edges()) {
			free_data();
			return EXIT_FAILURE;
		}
		if (!read_street(street)) {
			free_data();
			return EXIT_FAILURE;
		}
	}
	printf("Number of paths %lu\n", n_paths);
	for (node = nodes; node < node_out && node->n_paths; node++);
	if (node < node_out) {
		fprintf(stderr, "No solution\n");
		free_data();
		return EXIT_FAILURE;
	}
	q_nodes = malloc(sizeof(node_t *)*n_nodes);
	if (!q_nodes) {
		fprintf(stderr, "Cannot allocate memory for queue nodes\n");
		free_data();
		return EXIT_FAILURE;
	}
	positive = positive_node();
	while (positive < node_out) {
		printf("Reduce polarity from ");
		print_node(positive);
		puts("");
		for (node = nodes; node < node_out; node++) {
			node->visited = 0;
		}
		positive->visited = 1;
		*q_nodes = positive;
		q_node_out = q_nodes+1;
		for (q_node = q_nodes; q_node < q_node_out && !negative_polarity(*q_node); q_node++) {
			add_q_nodes(*q_node);
		}
		if (q_node < q_node_out) {
			for (node = *q_node; node->from != positive; node = node->from) {
				if (!add_path(node->from, node)) {
					free_data();
					return EXIT_FAILURE;
				}
			}
			if (!add_path(positive, node)) {
				free_data();
				return EXIT_FAILURE;
			}
		}
		else {
			fprintf(stderr, "Cannot reduce polarity\n");
			free_data();
			return EXIT_FAILURE;
		}
		positive = positive_node();
	}
	free(q_nodes);
	q_nodes = NULL;
	printf("Number of paths after polarity reducing %lu\n", n_paths);
	q_nodes = malloc(sizeof(node_t *)*n_paths);
	if (!q_nodes) {
		fprintf(stderr, "Cannot allocate memory for queue nodes\n");
		free_data();
		return EXIT_FAILURE;
	}
	q_node_out = q_nodes+n_paths;
	q_node_top = q_nodes;
	n_calls = n_paths*6;
	calls = malloc(sizeof(call_t)*n_calls);
	if (!calls) {
		fprintf(stderr, "Cannot allocate memory for calls\n");
		free_data();
		return EXIT_FAILURE;
	}
	call_top = calls-1;
	search_circuit(nodes+(start_street-1)*n_streets+start_avenue-1);
	free(calls);
	free_data();
	return EXIT_SUCCESS;
}

int read_street(unsigned long street) {
const int edge_types[N_EDGE_TYPES] = { '-', '<', '>', 'o' };
unsigned long avenue;
	if (!read_node(street, 1UL)) {
		return 0;
	}
	for (avenue = 2; avenue <= n_avenues; avenue++) {
		if (!read_separator()) {
			return 0;
		}
		if (!read_edge(edge_types, N_EDGE_TYPES)) {
			return 0;
		}
		if (!read_separator()) {
			return 0;
		}
		if (!read_node(street, avenue)) {
			return 0;
		}
	}
	fgetc(stdin);
	return 1;
}

int read_node(unsigned long street, unsigned long avenue) {
int *edge;
node_t *node;
	if (fgetc(stdin) != 'o') {
		fprintf(stderr, "Invalid node\n");
		return 0;
	}
	current_node->street = street;
	current_node->avenue = avenue;
	current_node->n_paths = 0;
	current_node->polarity = 0;
	if (street > 1) {
		edge = current_edge-n_avenues;
		node = current_node-n_avenues;
		if (*edge == '|') {
			if (!add_path(current_node, node)) {
				return 0;
			}
			if (!add_path(node, current_node)) {
				return 0;
			}
		}
		else if (*edge == '^') {
			if (!add_path(current_node, node)) {
				return 0;
			}
		}
		else if (*edge == 'v') {
			if (!add_path(node, current_node)) {
				return 0;
			}
		}
	}
	if (avenue > 1) {
		edge = current_edge-1;
		node = current_node-1;
		if (*edge == '-') {
			if (!add_path(current_node, node)) {
				return 0;
			}
			if (!add_path(node, current_node)) {
				return 0;
			}
		}
		else if (*edge == '<') {
			if (!add_path(current_node, node)) {
				return 0;
			}
		}
		else if (*edge == '>') {
			if (!add_path(node, current_node)) {
				return 0;
			}
		}
	}
	current_node++;
	return 1;
}

path_t *add_path(node_t *from, node_t *to) {
path_t *tmp;
	if (from->n_paths) {
		tmp = realloc(from->paths, sizeof(path_t)*(from->n_paths+1));
		if (!tmp) {
			fprintf(stderr, "Cannot reallocate memory for paths\n");
			from->n_paths = 0;
			free(from->paths);
			return NULL;
		}
		from->paths = tmp;
	}
	else {
		from->paths = malloc(sizeof(path_t));
		if (!from->paths) {
			fprintf(stderr, "Cannot allocate memory for paths\n");
			return NULL;
		}
	}
	set_path(from->paths+from->n_paths, to);
	from->n_paths++;
	from->path_out = from->paths+from->n_paths;
	from->polarity--;
	to->polarity++;
	n_paths++;
	return from->paths;
}

void set_path(path_t *path, node_t *to) {
	path->to = to;
	path->visited = 0;
}

int read_edges(void) {
const int edge_types[N_EDGE_TYPES] = { '|', '^', 'v', 'o' };
unsigned long avenue;
	if (!read_edge(edge_types, N_EDGE_TYPES)) {
		return 0;
	}
	for (avenue = 2; avenue <= n_avenues; avenue++) {
		if (!read_separator() || !read_separator() || !read_separator()) {
			return 0;
		}
		if (!read_edge(edge_types, N_EDGE_TYPES)) {
			return 0;
		}
	}
	fgetc(stdin);
	return 1;
}

int read_edge(const int *types, unsigned long n_types) {
const int *type_out = types+n_types, *type;
int type_read = fgetc(stdin);
	for (type = types; type < type_out && *type != type_read; type++);
	if (type == type_out) {
		fprintf(stderr, "Invalid edge\n");
		return 0;
	}
	*current_edge++ = type_read;
	return 1;
}

int read_separator(void) {
	if (fgetc(stdin) != ' ') {
		fprintf(stderr, "Invalid separator\n");
		return 0;
	}
	return 1;
}

node_t *positive_node(void) {
node_t *node;
	for (node = nodes; node < node_out && node->polarity <= 0; node++);
	return node;
}

int negative_polarity(node_t *node) {
	return node->polarity < 0;
}

void add_q_nodes(node_t *node) {
path_t *path;
	for (path = node->paths; path < node->path_out; path++) {
		if (!path->to->visited) {
			path->to->visited = 1;
			path->to->from = node;
			*q_node_out++ = path->to;
		}
	}
}

void search_circuit(node_t *start) {
int found = 0, type;
path_t *path;
node_t *node, **q_node;
	add_call(1, start, NULL);
	while (call_top >= calls && !found) {
		type = call_top->type;
		node = call_top->node;
		path = call_top->path;
		call_top--;
		if (type == 1) {
			if (q_node_top < q_node_out) {
				for (path = node->paths; path < node->path_out; path++) {
					if (!path->visited) {
						add_call(3, NULL, path);
						add_call(1, path->to, NULL);
						add_call(2, NULL, path);
					}
				}
			}
			else {
				printf("Circuit ");
				print_node(start);
				for (q_node = q_nodes; q_node < q_node_out; q_node++) {
					putchar(' ');
					print_node(*q_node);
				}
				puts("");
				found = 1;
			}
		}
		else if (type == 2) {
			path->visited = 1;
			*q_node_top++ = path->to;
		}
		else {
			q_node_top--;
			path->visited = 0;
		}
	}
}

void add_call(int type, node_t *node, path_t *path) {
	call_top++;
	call_top->type = type;
	call_top->node = node;
	call_top->path = path;
}

void print_node(node_t *node) {
	printf("S%luA%lu", node->street, node->avenue);
}

void free_data(void) {
node_t *node;
	if (q_nodes) {
		free(q_nodes);
	}
	if (nodes) {
		for (node = nodes; node < node_out; node++) {
			if (node->n_paths) {
				free(node->paths);
			}
		}
		free(nodes);
	}
	if (edges) {
		free(edges);
	}
}
