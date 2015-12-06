#include <stdio.h>
#include <stdlib.h>

#define N_EDGE_TYPES 4UL

struct edge_s {
	int type;
	int visited;
};
typedef struct edge_s edge_t;

typedef struct node_s node_t;

struct path_s {
	edge_t *edge;
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
	unsigned long n_initial_visited;
	unsigned long n_visited;
};
typedef struct call_s call_t;

int read_street(unsigned long);
int read_node(unsigned long, unsigned long);
path_t *add_path(node_t *, edge_t *, node_t *);
void set_path(path_t *, edge_t *, node_t *);
int read_edges(void);
int read_edge(const int *, unsigned long);
int read_separator(void);
node_t *positive_node(void);
int negative_polarity(node_t *);
void add_q_nodes(node_t *);
void search_circuit(node_t *, int);
void add_call(int, node_t *, path_t *, unsigned long, unsigned long);
void print_circuit(node_t *, unsigned long, unsigned long *);
void print_node(node_t *);
void free_data(void);

unsigned long n_avenues, n_open_edges, n_initial_paths, n_paths;
edge_t *edges = NULL, *current_edge;
node_t *nodes = NULL, *node_out, *current_node, **q_nodes = NULL, **q_node_out;
call_t *calls = NULL, *call_top;

int main(void) {
int manhattan;
unsigned long n_streets, start_street, start_avenue, n_edges, n_nodes, street, n_calls;
node_t *node, *positive, **q_node, **q_nodes_tmp;
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
	edges = malloc(sizeof(edge_t)*n_edges);
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
	n_open_edges = 0;
	n_initial_paths = 0;
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
	if (scanf("%d", &manhattan) != 1 || manhattan < 0 || manhattan > 1) {
		fprintf(stderr, "Invalid Manhattan flag\n");
		free_data();
		return EXIT_FAILURE;
	}
	printf("Number of initial paths %lu\n", n_initial_paths);
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
				if (!add_path(node->from, NULL, node)) {
					free_data();
					return EXIT_FAILURE;
				}
			}
			if (!add_path(positive, NULL, node)) {
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
	printf("Number of paths after polarity reducing %lu\n", n_paths);
	q_nodes_tmp = realloc(q_nodes, sizeof(node_t *)*n_paths);
	if (!q_nodes_tmp) {
		fprintf(stderr, "Cannot reallocate memory for queue nodes\n");
		free_data();
		return EXIT_FAILURE;
	}
	q_nodes = q_nodes_tmp;
	q_node_out = q_nodes;
	n_calls = n_paths*6;
	calls = malloc(sizeof(call_t)*n_calls);
	if (!calls) {
		fprintf(stderr, "Cannot allocate memory for calls\n");
		free_data();
		return EXIT_FAILURE;
	}
	call_top = calls-1;
	search_circuit(nodes+(start_street-1)*n_streets+start_avenue-1, manhattan);
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
edge_t *edge;
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
		if (edge->type == '|') {
			if (!add_path(current_node, edge, node)) {
				return 0;
			}
			if (!add_path(node, edge, current_node)) {
				return 0;
			}
		}
		else if (edge->type == '^') {
			if (!add_path(current_node, edge, node)) {
				return 0;
			}
		}
		else if (edge->type == 'v') {
			if (!add_path(node, edge, current_node)) {
				return 0;
			}
		}
	}
	if (avenue > 1) {
		edge = current_edge-1;
		node = current_node-1;
		if (edge->type == '-') {
			if (!add_path(current_node, edge, node)) {
				return 0;
			}
			if (!add_path(node, edge, current_node)) {
				return 0;
			}
		}
		else if (edge->type == '<') {
			if (!add_path(current_node, edge, node)) {
				return 0;
			}
		}
		else if (edge->type == '>') {
			if (!add_path(node, edge, current_node)) {
				return 0;
			}
		}
	}
	current_node++;
	return 1;
}

path_t *add_path(node_t *from, edge_t *edge, node_t *to) {
path_t *paths_tmp;
	if (from->n_paths) {
		paths_tmp = realloc(from->paths, sizeof(path_t)*(from->n_paths+1));
		if (!paths_tmp) {
			fprintf(stderr, "Cannot reallocate memory for paths\n");
			from->n_paths = 0;
			free(from->paths);
			return NULL;
		}
		from->paths = paths_tmp;
	}
	else {
		from->paths = malloc(sizeof(path_t));
		if (!from->paths) {
			fprintf(stderr, "Cannot allocate memory for paths\n");
			return NULL;
		}
	}
	set_path(from->paths+from->n_paths, edge, to);
	from->n_paths++;
	from->path_out = from->paths+from->n_paths;
	from->polarity--;
	to->polarity++;
	if (edge) {
		n_initial_paths++;
	}
	n_paths++;
	return from->paths;
}

void set_path(path_t *path, edge_t *edge, node_t *to) {
	path->edge = edge;
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
	if (type_read != 'o') {
		n_open_edges++;
	}
	current_edge->type = type_read;
	current_edge->visited = 0;
	current_edge++;
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

void search_circuit(node_t *start, int manhattan) {
int type;
unsigned long n_visited_min = n_paths, n_initial_visited, n_visited;
path_t *path;
node_t *node;
	add_call(1+manhattan, start, NULL, 0UL, 0UL);
	while (call_top >= calls) {
		type = call_top->type;
		node = call_top->node;
		path = call_top->path;
		n_initial_visited = call_top->n_initial_visited;
		n_visited = call_top->n_visited;
		call_top--;
		if (type == 1) {
			if (n_initial_visited < n_open_edges || *(q_node_out-1) != start) {
				if (n_visited < n_visited_min) {
					for (path = node->paths; path < node->path_out; path++) {
						if (path->edge && !path->edge->visited) {
							add_call(5, NULL, path, n_initial_visited+1, n_visited+1);
							add_call(1, path->to, NULL, n_initial_visited+1, n_visited+1);
							add_call(3, NULL, path, n_initial_visited+1, n_visited+1);
						}
						else if (!path->visited) {
							add_call(5, NULL, path, n_initial_visited, n_visited+1);
							add_call(1, path->to, NULL, n_initial_visited, n_visited+1);
							add_call(3, NULL, path, n_initial_visited, n_visited+1);
						}
					}
				}
			}
			else {
				print_circuit(start, n_visited, &n_visited_min);
			}
		}
		else if (type == 2) {
			if (n_initial_visited < n_initial_paths || *(q_node_out-1) != start) {
				if (n_visited < n_visited_min) {
					for (path = node->paths; path < node->path_out; path++) {
						if (!path->visited) {
							if (path->edge) {
								add_call(6, NULL, path, n_initial_visited+1, n_visited+1);
								add_call(2, path->to, NULL, n_initial_visited+1, n_visited+1);
								add_call(4, NULL, path, n_initial_visited+1, n_visited+1);
							}
							else {
								add_call(6, NULL, path, n_initial_visited, n_visited+1);
								add_call(2, path->to, NULL, n_initial_visited, n_visited+1);
								add_call(4, NULL, path, n_initial_visited, n_visited+1);
							}
						}
					}
				}
			}
			else {
				print_circuit(start, n_visited, &n_visited_min);
			}
		}
		else if (type == 3) {
			if (path->edge) {
				path->edge->visited++;
			}
			path->visited = 1;
			*q_node_out++ = path->to;
		}
		else if (type == 4) {
			path->visited = 1;
			*q_node_out++ = path->to;
		}
		else if (type == 5) {
			q_node_out--;
			path->visited = 0;
			if (path->edge) {
				path->edge->visited--;
			}
		}
		else {
			q_node_out--;
			path->visited = 0;
		}
	}
}

void add_call(int type, node_t *node, path_t *path, unsigned long n_initial_visited, unsigned long n_visited) {
	call_top++;
	call_top->type = type;
	call_top->node = node;
	call_top->path = path;
	call_top->n_initial_visited = n_initial_visited;
	call_top->n_visited = n_visited;
}

void print_circuit(node_t *start, unsigned long n_visited, unsigned long *n_visited_min) {
node_t **q_node;
	printf("Circuit ");
	print_node(start);
	for (q_node = q_nodes; q_node < q_node_out; q_node++) {
		putchar(' ');
		print_node(*q_node);
	}
	printf("\nLength %lu\n", n_visited);
	if (*n_visited_min > n_visited) {
		*n_visited_min = n_visited;
	}
}

void print_node(node_t *node) {
	printf("S%lu/A%lu", node->street, node->avenue);
}

void free_data(void) {
node_t *node;
	if (calls) {
		free(calls);
	}
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
