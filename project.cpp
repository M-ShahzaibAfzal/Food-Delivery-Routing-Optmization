#include <iostream>
#include <fstream>
#include <cmath>
#include <cstring>
#include <string>

using namespace std;

const int MAX_GRID_SIZE = 100;
const int MAX_ORDERS = 100;

// Custom queue for BFS
template<typename T>
class Queue {
private:
    struct Node {
        T data;
        Node* next;
        Node(T value) : data(value), next(nullptr) {}
    };

    Node* front;
    Node* rear;

public:
    Queue() : front(nullptr), rear(nullptr) {}

    bool empty() const {
        return front == nullptr;
    }

    void enqueue(T value) {
        Node* newNode = new Node(value);
        if (empty()) {
            front = rear = newNode;
        }
        else {
            rear->next = newNode;
            rear = newNode;
        }
    }

    T dequeue() {
        if (empty()) {
            throw runtime_error("Queue is empty");
        }

        Node* temp = front;
        T data = front->data;
        front = front->next;
        if (front == nullptr) {
            rear = nullptr;
        }
        delete temp;
        return data;
    }

    T Front() const {
        if (empty()) {
            throw runtime_error("Queue is empty");
        }
        return front->data;
    }
};

// Represents an order with location and delivery time limit
class Order {
public:
    string name;
    int location;
    int timeLimit;

    Order() : name(""), location(0), timeLimit(0) {}

    Order(const string& name, int loc, int limit)
        : name(name), location(loc), timeLimit(limit) {}
};

// Represents a restaurant with its location and a list of orders
class Restaurant {
public:
    string name;
    int location;
    Order orders[MAX_ORDERS];  // Static array for orders
    int orderCount;

    Restaurant() : name(""), location(0), orderCount(0) {}

    void addOrder(const Order& order) {
        if (orderCount < MAX_ORDERS) {
            orders[orderCount++] = order;
        }
    }
};

// Represents a node in the grid city
class CityNode {
public:
    int id;
    int connections[MAX_GRID_SIZE];
    int connectionCount;

    CityNode() : id(0), connectionCount(0) {
        memset(connections, 0, sizeof(connections));
    }

    CityNode(int id) : id(id), connectionCount(0) {
        memset(connections, 0, sizeof(connections));
    }

    void addConnection(int destination) {
        if (connectionCount < MAX_GRID_SIZE) {
            connections[connectionCount++] = destination;
        }
    }
};

// Represents the grid city with road connections
class GridCity {
public:
    CityNode nodes[MAX_GRID_SIZE * MAX_GRID_SIZE];  // Array of city nodes
    int gridSize;

    GridCity(int gridSize) : gridSize(gridSize) {
        int totalNodes = gridSize * gridSize;

        for (int i = 0; i < totalNodes; ++i) {
            nodes[i] = CityNode(i + 1);  // Properly initialized
        }

        // Create road connections in a grid-like pattern
        for (int i = 0; i < totalNodes; ++i) {
            if ((i + 1) % gridSize != 0) {
                nodes[i].addConnection(i + 2);  // Connect to the right
            }
            if (i + gridSize < totalNodes) {
                nodes[i].addConnection(i + gridSize + 1);  // Connect downward
            }
        }
    }

    void displayGraph(const Restaurant* restaurants, int restaurantCount) {
        cout << "Grid City Graph:" << endl;
        for (int i = 0; i < gridSize; ++i) {
            for (int j = 0; j < gridSize; ++j) {
                int nodeId = i * gridSize + j + 1;
                bool isRestaurant = false;
                for (int k = 0; k < restaurantCount; ++k) {
                    if (restaurants[k].location == nodeId) {
                        cout << restaurants[k].name[0];  // Display first character of restaurant name
                        isRestaurant = true;
                        break;
                    }
                }
                if (!isRestaurant) {
                    cout << nodeId;
                }
                if (j < gridSize - 1) {
                    cout << " -- ";
                }
            }
            cout << endl;
            if (i < gridSize - 1) {
                for (int j = 0; j < gridSize; ++j) {
                    int nodeId = i * gridSize + j + 1;
                    int rightNodeId = nodeId + 1;
                    if (rightNodeId <= gridSize * (i + 1)) {
                        cout << "|    ";
                    }
                }
                cout << endl;
            }
        }
        cout << endl;
    }
};

// BFS traversal to find the shortest path
bool bfs(const GridCity& city, int start, int target) {
    Queue<int> queue;
    bool visited[MAX_GRID_SIZE * MAX_GRID_SIZE] = { false };

    queue.enqueue(start);
    visited[start - 1] = true;  // Mark as visited

    while (!queue.empty()) {
        int currentNode = queue.dequeue();
        if (currentNode == target) {
            return true;  // Path found
        }

        // Explore connections from currentNode
        CityNode node = city.nodes[currentNode - 1];  // Adjust for zero-based indexing
        for (int i = 0; i < node.connectionCount; ++i) {
            int nextNode = node.connections[i];
            if (!visited[nextNode - 1]) {
                queue.enqueue(nextNode);
                visited[nextNode - 1] = true;  // Mark as visited
            }
        }
    }

    return false;  // No path found
}

// Calculate minimum delivery time considering BFS traversal
int calculateMinimumDeliveryTime(const GridCity& city, const Restaurant* restaurants, int restaurantCount, int* riderInitialLocations, int* riderMinTimeUnits) {
    int totalMinTime = 0;

    for (int i = 0; i < restaurantCount; ++i) {
        const Restaurant& restaurant = restaurants[i];
        int riderTime = 0;
        riderInitialLocations[i] = restaurant.location;  // Store rider's initial location

        for (int j = 0; j < restaurant.orderCount; ++j) {
            const Order& order = restaurant.orders[j];
            int travelTime = abs(order.location - restaurant.location);

            if (travelTime > order.timeLimit) {
                cout << "I am Sorry for disturbing you. Delivery to " << order.name << " from " << restaurant.name << " is not possible within the time limit." << endl;
                return -1;  // If any order can't be delivered within its limit, it's invalid
            }

            riderTime = max(riderTime, travelTime);  // Track rider's travel time
        }

        cout << "Rider " << i + 1 << ": " << riderInitialLocations[i] << " -> ";
        cout << " = " << riderTime << " time units" << endl;

        riderMinTimeUnits[i] = riderTime;  // Store rider's minimum time units
        totalMinTime += riderTime;  // Sum up rider times for total time
    }

    cout << "Total: " << totalMinTime << " time units" << endl;
    return totalMinTime;  // Return the total minimum time required to complete all deliveries
}

// Main menu for handling test cases and user input
class Menu {
public:
    static void displayMenu(ifstream& inputFile) {
        cout << " Welcome to the Islamabad City Delivery Program :) ." << endl;

        int numTestCases = 0;
        inputFile >> numTestCases;

        if (numTestCases <= 0) {
            cout << "Invalid number of test cases. Enter a positive integer." << endl;
            return;
        }

        while (numTestCases-- > 0) {
            int gridSize = 0, numRiders = 0, numRestaurants = 0;

            inputFile >> gridSize >> numRiders >> numRestaurants;

            GridCity city(gridSize);

            Restaurant* restaurants = new Restaurant[MAX_GRID_SIZE];  // Allocate on heap
            int restaurantCount = 0;

            for (int i = 0; i < numRestaurants; ++i) {  // Process each restaurant
                string restaurantName;
                int location = 0, numOrders = 0;

                inputFile >> restaurantName >> location >> numOrders;

                restaurants[restaurantCount].name = restaurantName;
                restaurants[restaurantCount].location = location;

                for (int j = 0; j < numOrders; ++j) {  // Process each order
                    string orderName;
                    int orderLocation = 0, timeLimit = 0;

                    inputFile >> orderName >> orderLocation >> timeLimit;

                    Order order(orderName, orderLocation, timeLimit);
                    restaurants[restaurantCount].addOrder(order);
                }

                restaurantCount++;
            }

            int* riderInitialLocations = new int[numRiders];  // Array to store rider's initial locations
            int* riderMinTimeUnits = new int[numRiders];  // Array to store rider's minimum time units

            city.displayGraph(restaurants, restaurantCount);  // Display the grid city graph

            int totalMinTime = calculateMinimumDeliveryTime(city, restaurants, restaurantCount, riderInitialLocations, riderMinTimeUnits);

            if (totalMinTime != -1) {
                cout << "Total: " << totalMinTime << " time units" << endl;
            }

            delete[] restaurants;  // Deallocate memory
            delete[] riderInitialLocations;  // Deallocate rider initial locations array
            delete[] riderMinTimeUnits;  // Deallocate rider minimum time units array
        }
    }
};

int main() {
    ifstream inputFile("input.txt");
    if (!inputFile.is_open()) {
        cout << "Failed to open input file." << endl;
        return 1;
    }

    Menu::displayMenu(inputFile);

    inputFile.close();
    return 0;
}

