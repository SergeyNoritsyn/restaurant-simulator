#include "restaurant.h"
/**
	Add your functions to this file.
	Make sure to review a1.h.
	Do NOT include a main() function in this file
	when you submit.
*/

Restaurant* initialize_restaurant(char* name){
	Restaurant* new_restaurant = (Restaurant *)malloc(sizeof(Restaurant));
	new_restaurant->name = (char *)malloc(sizeof(char) * (strlen(name) + 1));
	strcpy(new_restaurant->name, name);
	new_restaurant->menu = load_menu(MENU_FNAME);
	new_restaurant->num_completed_orders = 0;
	new_restaurant->num_pending_orders = 0;
	new_restaurant->pending_orders = (Queue *)malloc(sizeof(Queue));
	new_restaurant->pending_orders->head = NULL;
	new_restaurant->pending_orders->tail = NULL;
	return new_restaurant;
}

Menu* load_menu(char* fname){
	Menu* menu = (Menu *)malloc(sizeof(Menu));
	menu->num_items = 0;
	int max_items = 1;
	int str_len = ITEM_CODE_LENGTH + MAX_ITEM_NAME_LENGTH + 30;
	char line[str_len];
	menu->item_codes = (char **)malloc(sizeof(char *) * max_items);
	menu->item_names = (char **)malloc(sizeof(char *) * max_items);
	menu->item_cost_per_unit = (double *)malloc(sizeof(double) * max_items);
	FILE* fp = fopen(fname, "r");
	while(fgets(line, str_len, fp)){
		// check if the line is whitespace-only
		int flag = 0;
		for (int i = 0; i < strlen(line); i++)
		{
			if (line[i] != '\r' && line[i] != '\n' && line[i] != ' ' && line[i] != '\t')
			{
				flag = 1;
				break;
			}
		}

		if (flag == 0)
		{
			break;
		}

		if (menu->num_items == max_items){
			max_items *= 2;
			menu->item_codes = realloc(menu->item_codes, sizeof(char *) * max_items);
			menu->item_names = realloc(menu->item_names, sizeof(char *) * max_items);
			menu->item_cost_per_unit = realloc(menu->item_cost_per_unit, sizeof(double *) * max_items);
		}
		*(menu->item_codes + menu->num_items) = (char *)malloc(sizeof(char) * ITEM_CODE_LENGTH);
		*(menu->item_names + menu->num_items) = (char *)malloc(sizeof(char) * MAX_ITEM_NAME_LENGTH);
		char *token = strtok(line, MENU_DELIM);
		while (token[0] == ' '){ // removing leading whitespace
			token = token + 1;
		}
		strcpy(*(menu->item_codes + menu->num_items), token);
		token = strtok(NULL, MENU_DELIM);
		strcpy(*(menu->item_names + menu->num_items), token);
		token = strtok(NULL, MENU_DELIM);
		char money_double[strlen(token)];
		strncpy(money_double, &token[1], strlen(token) - 1);
		char *temp; // stores any potential trailing whitespace
		// conversion to price format - dollars and two digits after decimal point for cents (rounding)
		*(menu->item_cost_per_unit + menu->num_items) = (int)(strtod(money_double, &temp) * 100 + 0.5) / 100.0;
		menu->num_items++;
	}
	fclose(fp);
	return menu;
}

Order* build_order(char* items, char* quantities){
	Order *order = (Order *)malloc(sizeof(Order));
	order->num_items = strlen(items) / (ITEM_CODE_LENGTH - 1);
	order->item_codes = (char **)malloc(sizeof(char *) * order->num_items);
	order->item_quantities = (int *)malloc(sizeof(int) * order->num_items);
	char *temp = strdup(quantities);
	char *token = strtok(temp, MENU_DELIM);
	char copy_code[ITEM_CODE_LENGTH];
	for (int i = 0; i < order->num_items; i++){
		*(order->item_codes + i) = (char *)malloc(sizeof(char) * ITEM_CODE_LENGTH);
		strncpy(copy_code, &items[i * (ITEM_CODE_LENGTH - 1)], ITEM_CODE_LENGTH - 1);
		copy_code[2] = '\0';
		strcpy(*(order->item_codes + i), copy_code);
		*(order->item_quantities + i) = atoi(token);
		if (i != order->num_items - 1){
			token = strtok(NULL, MENU_DELIM);
		}
	}
	free(temp);
	temp = NULL;
	token = NULL;
	return order;
}

void enqueue_order(Order* order, Restaurant* restaurant){
	if (restaurant->num_pending_orders == 0){
		restaurant->pending_orders->head = (QueueNode *)malloc(sizeof(QueueNode));
		restaurant->pending_orders->head->order = order;
		restaurant->pending_orders->head->next = NULL;
		restaurant->pending_orders->tail = NULL;
	}
	else if (restaurant->num_pending_orders == 1){
		restaurant->pending_orders->tail = (QueueNode *)malloc(sizeof(QueueNode));
		restaurant->pending_orders->tail->order = order;
		restaurant->pending_orders->tail->next = NULL;
		restaurant->pending_orders->head->next = restaurant->pending_orders->tail;
	}
	else
	{
		QueueNode *temp = restaurant->pending_orders->tail;
		temp->next = (QueueNode *)malloc(sizeof(QueueNode));
		temp->next->order = order;
		restaurant->pending_orders->tail = temp->next;
	}
	restaurant->num_pending_orders++;
}

Order* dequeue_order(Restaurant* restaurant){
	QueueNode *temp = restaurant->pending_orders->head;
	Order *temp_order = temp->order;
	if (restaurant->num_pending_orders == 1){
		restaurant->pending_orders->head = temp->next;
		if (restaurant->pending_orders->tail != NULL)
		{
			restaurant->pending_orders->tail = temp->next;
		}
	}
	else{
		restaurant->pending_orders->head = temp->next;
	}
	restaurant->num_completed_orders++;
	restaurant->num_pending_orders--;
	free(temp);
	temp = NULL;
	return temp_order;
}

double get_item_cost(char* item_code, Menu* menu){
	int index = 0;
	for (int i = 0; i < menu->num_items; i++){
		if (strcmp(item_code, *(menu->item_codes + i)) == 0){
			break;
		}
		index++;
	}
	return *(menu->item_cost_per_unit + index);
}

double get_order_subtotal(Order* order, Menu* menu){
	double subtotal = 0;
	for (int i = 0; i < order->num_items; i++){
		subtotal += get_item_cost(*(order->item_codes + i), menu) * (*(order->item_quantities + i));
	}
	return subtotal;
}

double get_order_total(Order* order, Menu* menu){
	return get_order_subtotal(order, menu) * (TAX_RATE / 100.0 + 1);
}

int get_num_completed_orders(Restaurant* restaurant){
	return restaurant->num_completed_orders;
}

int get_num_pending_orders(Restaurant* restaurant){
	return restaurant->num_pending_orders;
}

void clear_order(Order** order){
	for (int i = 0; i < (*order)->num_items; i++){
		free(*((*order)->item_codes + i));
	}
	free((*order)->item_codes);
	free((*order)->item_quantities);
	free(*order);
	*order = NULL;
}

void clear_menu(Menu** menu){
	for (int i = 0; i < (*menu)->num_items; i++){
		free(*((*menu)->item_codes + i));
		free(*((*menu)->item_names + i));
	}
	free((*menu)->item_cost_per_unit);
	free((*menu)->item_codes);
	free((*menu)->item_names);
	free(*menu);
	*menu = NULL;
}

void close_restaurant(Restaurant** restaurant){
	clear_menu(&((*restaurant)->menu));
	free((*restaurant)->name);
	while ((*restaurant)->num_pending_orders > 0){
		Order *order = dequeue_order(*restaurant);
		clear_order(&order);
		(*restaurant)->num_pending_orders--;
		(*restaurant)->num_completed_orders++;
	}
	free((*restaurant)->pending_orders);
	free(*restaurant);
	*restaurant = NULL;
}

void print_menu(Menu* menu){
	fprintf(stdout, "--- Menu ---\n");
	for (int i = 0; i < menu->num_items; i++){
		fprintf(stdout, "(%s) %s: %.2f\n", 
			menu->item_codes[i], 
			menu->item_names[i], 
			menu->item_cost_per_unit[i]	
		);
	}
}


void print_order(Order* order){
	for (int i = 0; i < order->num_items; i++){
		fprintf(
			stdout, 
			"%d x (%s)\n", 
			order->item_quantities[i], 
			order->item_codes[i]
		);
	}
}


void print_receipt(Order* order, Menu* menu){
	for (int i = 0; i < order->num_items; i++){
		double item_cost = get_item_cost(order->item_codes[i], menu);
		fprintf(
			stdout, 
			"%d x (%s)\n @$%.2f ea \t %.2f\n", 
			order->item_quantities[i],
			order->item_codes[i], 
			item_cost,
			item_cost * order->item_quantities[i]
		);
	}
	double order_subtotal = get_order_subtotal(order, menu);
	double order_total = get_order_total(order, menu);
	
	fprintf(stdout, "Subtotal: \t %.2f\n", order_subtotal);
	fprintf(stdout, "               -------\n");
	fprintf(stdout, "Tax %d%%: \t$%.2f\n", TAX_RATE, order_total);
	fprintf(stdout, "              ========\n");
}
