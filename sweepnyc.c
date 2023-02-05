#include <stdio.h>
#include <stdlib.h>

#define N_EDGE_TYPES 4
#define MAX_EVALUATIONS 4

typedef struct path_s path_t;
typedef struct node_s node_t;

typedef struct {
	int type;
	int visited;
}
edge_t;

struct path_s {
	node_t *from;
	edge_t *edge;
	node_t *to;
	int visited;
	path_t *last;
	path_t *next;
};

struct node_s {
	int street;
	int avenue;
	int n_paths;
	path_t *paths;
	int polarity;
	int n_visits;
	int visited;
	node_t *from;
	int distance;
};

typedef struct {
	int type;
	node_t *start;
	path_t *path;
}
call_t;

typedef struct {
	path_t *path;
	int distance;
	int rank;
	int call1;
	int call3;
}
evaluation_t;

static int read_street(int);
static int read_node(int, int);
static int link_node(edge_t *, node_t *, int, int, int);
static int read_edges(void);
static int read_edge(const int *, int);
static int read_separator(void);
static int reduce_polarity(node_t *);
static int add_path(node_t *, edge_t *, node_t *);
static void set_path(path_t *, node_t *, edge_t *, node_t *);
static void process_call(call_t *);
static void add_node_calls(node_t *, node_t *);
static void add_path_calls_chinese1(path_t *);
static void add_path_calls_chinese2(path_t *);
static void add_path_calls_chinese3(node_t *, path_t *);
static void add_path_calls_manhattan1(path_t *);
static void add_path_calls_manhattan2(path_t *);
static void add_path_calls_manhattan3(node_t *, path_t *);
static void evaluate_path(node_t *, path_t *, node_t *);
void add_q_node(node_t *, node_t *);
void check_distance(path_t *, node_t *, int);
void add_evaluation(path_t *, node_t *, int, int, int, int);
static int compare_evaluations(const void *, const void *);
static void add_path_calls(node_t *, path_t *, int, int);
static void add_call(int, node_t *, path_t *);
static void link_paths(path_t *, path_t *);
static void check_low_visited(void);
static void print_node(const node_t *);
static void free_data(void);

static int n_avenues, n_nodes, n_open_edges, n_initial_paths, n_paths, manhattan, low_bound, n_choices, n_q_nodes, min_visited, n_visited, low_visited, n_calls, n_evaluations;
static edge_t *edges = NULL, *current_edge;
static path_t **q_paths = NULL;
static node_t *nodes = NULL, *current_node, **q_nodes = NULL;
static call_t *calls = NULL;
static evaluation_t evaluations[MAX_EVALUATIONS];
static void (*add_path_calls1)(path_t *), (*add_path_calls2)(path_t *), (*add_path_calls3)(node_t *, path_t *);

int main(void) {
	int n_streets, start_street, start_avenue, n_edges, i;
	path_t path;
	node_t *start;
	if (scanf("%d", &n_streets) != 1 || n_streets < 1) {
		fputs("Invalid number of streets\n", stderr);
		fflush(stderr);
		return EXIT_FAILURE;
	}
	if (scanf("%d", &n_avenues) != 1 || n_avenues < 1) {
		fputs("Invalid number of avenues\n", stderr);
		fflush(stderr);
		return EXIT_FAILURE;
	}
	if (scanf("%d", &start_street) != 1 || start_street < 1 || start_street > n_streets) {
		fputs("Invalid starting street\n", stderr);
		fflush(stderr);
		return EXIT_FAILURE;
	}
	if (scanf("%d", &start_avenue) != 1 || start_avenue < 1 || start_avenue > n_avenues) {
		fputs("Invalid starting avenue\n", stderr);
		fflush(stderr);
		return EXIT_FAILURE;
	}
	getchar();
	n_edges = n_streets*(n_streets-1)+n_avenues*(n_avenues-1);
	if (n_edges) {
		edges = malloc(sizeof(edge_t)*(size_t)n_edges);
		if (!edges) {
			fputs("Cannot allocate memory for edges\n", stderr);
			fflush(stderr);
			return EXIT_FAILURE;
		}
	}
	n_nodes = n_streets*n_avenues;
	nodes = malloc(sizeof(node_t)*(size_t)n_nodes);
	if (!nodes) {
		fputs("Cannot allocate memory for nodes\n", stderr);
		fflush(stderr);
		free_data();
		return EXIT_FAILURE;
	}
	start = nodes+(start_street-1)*n_avenues+start_avenue-1;
	n_open_edges = 0;
	n_initial_paths = 0;
	n_paths = 0;
	current_edge = edges;
	current_node = nodes;
	if (!read_street(1)) {
		free_data();
		return EXIT_FAILURE;
	}
	for (i = 2; i <= n_streets; ++i) {
		if (!read_edges() || !read_street(i)) {
			free_data();
			return EXIT_FAILURE;
		}
	}
	if (scanf("%d", &manhattan) != 1 || manhattan < 0 || manhattan > 1) {
		fputs("Invalid Manhattan flag\n", stderr);
		fflush(stderr);
		free_data();
		return EXIT_FAILURE;
	}
	if (manhattan) {
		low_bound = n_initial_paths;
		add_path_calls1 = add_path_calls_manhattan1;
		add_path_calls2 = add_path_calls_manhattan2;
		add_path_calls3 = add_path_calls_manhattan3;
		printf("Number of initial paths %d\n", n_initial_paths);
	}
	else {
		low_bound = n_open_edges;
		add_path_calls1 = add_path_calls_chinese1;
		add_path_calls2 = add_path_calls_chinese2;
		add_path_calls3 = add_path_calls_chinese3;
		printf("Number of open edges %d\n", n_open_edges);
	}
	fflush(stdout);
	q_nodes = malloc(sizeof(node_t *)*(size_t)n_nodes);
	if (!q_nodes) {
		fputs("Cannot allocate memory for queue nodes\n", stderr);
		fflush(stderr);
		free_data();
		return EXIT_FAILURE;
	}
	for (i = 0; i < n_nodes && nodes[i].polarity <= 0; ++i);
	while (i < n_nodes) {
		if (!reduce_polarity(nodes+i)) {
			free_data();
			return EXIT_FAILURE;
		}
		for (; i < n_nodes && nodes[i].polarity <= 0; ++i);
	}
	printf("Number of paths after polarity reducing %d\n", n_paths);
	fflush(stdout);
	if (scanf("%d", &n_choices) != 1 || n_choices < 1 || n_choices > MAX_EVALUATIONS) {
		fputs("Invalid number of choices\n", stderr);
		fflush(stderr);
		free_data();
		return EXIT_FAILURE;
	}
	min_visited = n_paths+1;
	q_paths = malloc(sizeof(path_t *)*(size_t)min_visited);
	if (!q_paths) {
		fputs("Cannot allocate memory for q_paths\n", stderr);
		fflush(stderr);
		free_data();
		return EXIT_FAILURE;
	}
	set_path(&path, NULL, NULL, start);
	q_paths[0] = &path;
	n_visited = 0;
	calls = malloc(sizeof(call_t)*(size_t)(n_paths*6+1));
	if (!calls) {
		fputs("Cannot allocate memory for calls\n", stderr);
		fflush(stderr);
		free_data();
		return EXIT_FAILURE;
	}
	low_visited = n_paths;
	n_calls = 0;
	add_call(0, start, &path);
	while (n_calls) {
		--n_calls;
		process_call(calls+n_calls);
	}
	free_data();
	return EXIT_SUCCESS;
}

static int read_street(int street) {
	const int edge_types[N_EDGE_TYPES] = { '-', '<', '>', 'o' };
	int i;
	if (!read_node(street, 1)) {
		return 0;
	}
	for (i = 2; i <= n_avenues; ++i) {
		if (!read_separator() || !read_edge(edge_types, N_EDGE_TYPES) || !read_separator() || !read_node(street, i)) {
			return 0;
		}
	}
	getchar();
	return 1;
}

static int read_node(int street, int avenue) {
	if (getchar() != 'o') {
		fputs("Invalid node\n", stderr);
		fflush(stderr);
		return 0;
	}
	current_node->street = street;
	current_node->avenue = avenue;
	current_node->n_paths = 0;
	current_node->polarity = 0;
	current_node->n_visits = 0;
	current_node->visited = 0;
	if (street > 1 && !link_node(current_edge-n_avenues, current_node-n_avenues, '|', '^', 'v')) {
		return 0;
	}
	if (avenue > 1 && !link_node(current_edge-1, current_node-1, '-', '<', '>')) {
		return 0;
	}
	++current_node;
	return 1;
}

static int link_node(edge_t *edge, node_t *node, int type_d, int type_nw, int type_se) {
	if (edge->type == type_d) {
		if (!add_path(current_node, edge, node) || !add_path(node, edge, current_node)) {
			return 0;
		}
	}
	else if (edge->type == type_nw) {
		if (!add_path(current_node, edge, node)) {
			return 0;
		}
	}
	else if (edge->type == type_se) {
		if (!add_path(node, edge, current_node)) {
			return 0;
		}
	}
	return 1;
}

static int read_edges(void) {
	const int edge_types[N_EDGE_TYPES] = { '|', '^', 'v', 'o' };
	int i;
	if (!read_edge(edge_types, N_EDGE_TYPES)) {
		return 0;
	}
	for (i = 1; i < n_avenues; ++i) {
		if (!read_separator() || !read_separator() || !read_separator() || !read_edge(edge_types, N_EDGE_TYPES)) {
			return 0;
		}
	}
	getchar();
	return 1;
}

static int read_edge(const int *types, int n_types) {
	int type_read = getchar(), i;
	for (i = 0; i < n_types && types[i] != type_read; ++i);
	if (i < n_types) {
		if (type_read != 'o') {
			++n_open_edges;
		}
		current_edge->type = type_read;
		current_edge->visited = 0;
		++current_edge;
		return 1;
	}
	fputs("Invalid edge\n", stderr);
	fflush(stderr);
	return 0;
}

static int read_separator(void) {
	if (getchar() != ' ') {
		fputs("Invalid separator\n", stderr);
		fflush(stderr);
		return 0;
	}
	return 1;
}

static int reduce_polarity(node_t *positive) {
	int i, j;
	fputs("Reduce polarity from", stdout);
	print_node(positive);
	puts("");
	fflush(stdout);
	positive->visited = 1;
	q_nodes[0] = positive;
	n_q_nodes = 1;
	for (i = 0; i < n_q_nodes && q_nodes[i]->polarity >= 0; ++i) {
		int k;
		node_t *from = q_nodes[i];
		for (k = 0; k < from->n_paths; ++k) {
			node_t *to = from->paths[k].to;
			if (!to->visited) {
				to->visited = 1;
				to->from = from;
				q_nodes[n_q_nodes++] = to;
			}
		}
	}
	for (j = n_q_nodes; j--; ) {
		q_nodes[j]->visited = 0;
	}
	if (i < n_q_nodes) {
		node_t *node;
		for (node = q_nodes[i]; node->from != positive; node = node->from) {
			if (!add_path(node->from, NULL, node)) {
				return 0;
			}
		}
		return add_path(positive, NULL, node);
	}
	fputs("Cannot reduce polarity\n", stderr);
	fflush(stderr);
	return 0;
}

static int add_path(node_t *from, edge_t *edge, node_t *to) {
	if (from->n_paths) {
		path_t *paths_tmp = realloc(from->paths, sizeof(path_t)*(size_t)(from->n_paths+1));
		if (!paths_tmp) {
			fputs("Cannot reallocate memory for paths\n", stderr);
			fflush(stderr);
			from->n_paths = 0;
			free(from->paths);
			return 0;
		}
		from->paths = paths_tmp;
	}
	else {
		from->paths = malloc(sizeof(path_t));
		if (!from->paths) {
			fputs("Cannot allocate memory for paths\n", stderr);
			fflush(stderr);
			return 0;
		}
	}
	set_path(from->paths+from->n_paths, from, edge, to);
	++from->n_paths;
	--from->polarity;
	++to->polarity;
	if (edge) {
		++n_initial_paths;
	}
	++n_paths;
	return 1;
}

static void set_path(path_t *path, node_t *from, edge_t *edge, node_t *to) {
	path->from = from;
	path->edge = edge;
	path->to = to;
	path->visited = 0;
}

static void process_call(call_t *call) {
	int type = call->type;
	path_t *path = call->path;
	if (type == 0) {
		node_t *start = call->start, *node = path->to;
		if (low_bound || node != start) {
			if (n_visited+low_bound < min_visited) {
				int n_calls_bak = n_calls, i, j;
				if (!n_visited || node != start) {
					add_node_calls(start, node);
				}
				else {
					for (i = n_visited; i > 0; --i) {
						add_node_calls(q_paths[i]->from, q_paths[i]->from);
						if (min_visited <= n_paths || n_calls > n_calls_bak) {
							break;
						}
					}
				}
				for (i = n_calls_bak, j = n_calls-1; i < j; ++i, --j) {
					call_t tmp = calls[i];
					calls[i] = calls[j];
					calls[j] = tmp;
				}
			}
		}
		else {
			int i;
			for (i = 0; i < n_visited; ++i) {
				link_paths(q_paths[i], q_paths[i+1]);
			}
			link_paths(q_paths[i], q_paths[0]);
			for (path = q_paths[0]->last; path != q_paths[0]; path = path->last) {
				path_t *old_last = path->last;
				if (path->from != old_last->to) {
					path_t *new_last;
					for (new_last = old_last->last; new_last != q_paths[0] && new_last->to != path->from; new_last = new_last->last);
					link_paths(q_paths[0]->last, new_last->next);
					link_paths(new_last, path);
					link_paths(old_last, q_paths[0]);
					path = q_paths[0];
				}
			}
			fputs("Circuit", stdout);
			print_node(q_paths[0]->to);
			for (path = q_paths[0]->next; path != q_paths[0]; path = path->next) {
				print_node(path->to);
			}
			printf("\nLength %d\n", n_visited);
			fflush(stdout);
			min_visited = n_visited-1;
		}
	}
	else if (type == 1) {
		++path->from->n_visits;
		++path->edge->visited;
		path->visited = 1;
		--low_bound;
		q_paths[++n_visited] = path;
	}
	else if (type == 2) {
		++path->from->n_visits;
		path->visited = 1;
		q_paths[++n_visited] = path;
	}
	else if (type == 3) {
		--n_visited;
		++low_bound;
		path->visited = 0;
		--path->edge->visited;
		--path->from->n_visits;
		check_low_visited();
	}
	else {
		--n_visited;
		path->visited = 0;
		--path->from->n_visits;
		check_low_visited();
	}
}

static void add_node_calls(node_t *start, node_t *from) {
	int i;
	n_evaluations = 0;
	for (i = from->n_paths; i--; ) {
		add_path_calls1(from->paths+i);
	}
	for (i = from->n_paths; i--; ) {
		add_path_calls2(from->paths+i);
	}
	if (n_visited+1+low_bound < min_visited) {
		for (i = from->n_paths; i--; ) {
			add_path_calls3(start, from->paths+i);
		}
	}
	if (n_evaluations) {
		for (i = from->n_paths; i--; ) {
			node_t *to = from->paths[i].to;
			if (to->visited) {
				to->visited = 0;
			}
		}
		qsort(evaluations, (size_t)n_evaluations, sizeof(evaluation_t), compare_evaluations);
		for (i = 0; i < n_evaluations && i < n_choices; i++) {
			add_path_calls(start, evaluations[i].path, evaluations[i].call1, evaluations[i].call3);
		}
	}
}

static void add_path_calls_chinese1(path_t *path) {
	edge_t *edge = path->edge;
	node_t *to = path->to;
	if (edge && edge->type != '|' && edge->type != '-' && !edge->visited && !to->visited && !path->visited) {
		add_evaluation(path, to, 0, to->n_visits*2, 1, 3);
	}
}

static void add_path_calls_chinese2(path_t *path) {
	edge_t *edge = path->edge;
	node_t *to = path->to;
	if (edge && (edge->type == '|' || edge->type == '-') && !edge->visited && !to->visited && !path->visited) {
		add_evaluation(path, to, 0, to->n_visits*2+1, 1, 3);
	}
}

static void add_path_calls_chinese3(node_t *start, path_t *path) {
	edge_t *edge = path->edge;
	node_t *to = path->to;
	if ((!edge || edge->visited) && !to->visited && !path->visited) {
		evaluate_path(start, path, to);
	}
}

static void add_path_calls_manhattan1(path_t *path) {
	edge_t *edge = path->edge;
	node_t *to = path->to;
	if (edge && edge->type != '|' && edge->type != '-' && !to->visited && !path->visited) {
		add_evaluation(path, to, 0, to->n_visits*2, 1, 3);
	}
}

static void add_path_calls_manhattan2(path_t *path) {
	edge_t *edge = path->edge;
	node_t *to = path->to;
	if (edge && (edge->type == '|' || edge->type == '-') && !to->visited && !path->visited) {
		add_evaluation(path, to, 0, to->n_visits*2+1, 1, 3);
	}
}

static void add_path_calls_manhattan3(node_t *start, path_t *path) {
	node_t *to = path->to;
	if (!path->edge && !to->visited && !path->visited) {
		evaluate_path(start, path, to);
	}
}

static void evaluate_path(node_t *start, path_t *evaluated, node_t *to) {
	int i, j;
	to->visited = 2;
	to->distance = 1;
	q_nodes[0] = to;
	n_q_nodes = 1;
	if (low_bound) {
		for (i = 0; i < n_q_nodes; ++i) {
			node_t *from = q_nodes[i];
			int k;
			for (k = 0; k < from->n_paths; ++k) {
				path_t *path = from->paths+k;
				if (!path->visited) {
					if (manhattan) {
						if (!path->edge) {
							add_q_node(from, path->to);
						}
						else {
							break;
						}
					}
					else {
						if (!path->edge || path->edge->visited) {
							add_q_node(from, path->to);
						}
						else {
							break;
						}
					}
				}
			}
			if (k < from->n_paths) {
				break;
			}
		}
	}
	else {
		for (i = 0; i < n_q_nodes; ++i) {
			node_t *from = q_nodes[i];
			if (from != start) {
				int k;
				for (k = 0; k < from->n_paths; ++k) {
					path_t *path = from->paths+k;
					if (!path->visited) {
						add_q_node(from, path->to);
					}
				}
			}
			else {
				break;
			}
		}
	}
	for (j = n_q_nodes; j--; ) {
		q_nodes[j]->visited ^= 2;
	}
	if (i == n_q_nodes) {
		check_distance(evaluated, to, q_nodes[n_q_nodes-1]->distance);
	}
	else {
		check_distance(evaluated, to, q_nodes[i]->distance);
	}
}

void add_q_node(node_t *from, node_t *to) {
	if (!(to->visited & 2)) {
		to->visited |= 2;
		to->distance = from->distance+1;
		q_nodes[n_q_nodes++] = to;
	}
}

void check_distance(path_t *path, node_t *to, int distance) {
	if (n_visited+distance+low_bound < min_visited) {
		add_evaluation(path, to, distance, to->n_visits*2, 2, 4);
	}
}

void add_evaluation(path_t *path, node_t *to, int distance, int rank, int call1, int call3) {
	evaluation_t *evaluation = evaluations+n_evaluations;
	evaluation->path = path;
	evaluation->distance = distance;
	evaluation->rank = rank;
	evaluation->call1 = call1;
	evaluation->call3 = call3;
	n_evaluations++;
	to->visited = 1;
}

static int compare_evaluations(const void *a, const void *b) {
	const evaluation_t *evaluation_a = (const evaluation_t *)a, *evaluation_b = (const evaluation_t *)b;
	if (evaluation_a->distance != evaluation_b->distance) {
		return evaluation_a->distance-evaluation_b->distance;
	}
	return evaluation_a->rank-evaluation_b->rank;
}

static void add_path_calls(node_t *start, path_t *path, int type1, int type3) {
	add_call(type1, NULL, path);
	add_call(0, start, path);
	add_call(type3, NULL, path);
}

static void add_call(int type, node_t *start, path_t *path) {
	call_t *call = calls+n_calls;
	call->type = type;
	call->start = start;
	call->path = path;
	++n_calls;
}

static void link_paths(path_t *path_a, path_t *path_b) {
	path_a->next = path_b;
	path_b->last = path_a;
}

static void check_low_visited(void) {
	if (n_visited < low_visited) {
		low_visited = n_visited;
		printf("low_visited %d\n", low_visited);
		fflush(stdout);
	}
}

static void print_node(const node_t *node) {
	printf(" S%d/A%d", node->street, node->avenue);
}

static void free_data(void) {
	if (calls) {
		free(calls);
	}
	if (q_paths) {
		free(q_paths);
	}
	if (q_nodes) {
		free(q_nodes);
	}
	if (nodes) {
		int i;
		for (i = n_nodes; i--; ) {
			if (nodes[i].n_paths) {
				free(nodes[i].paths);
			}
		}
		free(nodes);
	}
	if (edges) {
		free(edges);
	}
}
