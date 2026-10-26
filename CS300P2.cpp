//============================================================================
// Name        : ProjectTwo.cpp
// Author      : Scottie Andino
// Version     : 1.0
// Course      : CS 300 DSA: Analysis and Design
// Description : ABCU advising assistance program. Loads the course data
//               file into a hash table, prints the courses in alphanumeric
//               order, and prints the title and prerequisites of one course.
//============================================================================

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

//============================================================================
// Global definitions visible to all methods and classes
//============================================================================

// A prime number of buckets spreads the course numbers out so there are
// fewer collisions
const unsigned int TABLE_SIZE = 179;

// A bucket with this key has never been used
const int UNUSED = -1;

// The sample output lists only the prerequisite course numbers. Change this
// to true to also print each prerequisite's title (one per line).
const bool SHOW_PREREQUISITE_TITLES = false;

// Holds the data from one line of the input file
struct Course {
    string courseNumber;
    string courseName;
    vector<string> prerequisites;
};

//============================================================================
// Hash Table class definition
//============================================================================

/**
 * A hash table that stores Course objects. Courses that hash to the same
 * bucket are chained together with next pointers (chaining).
 */
class HashTable {

private:
    // One bucket (or one link in a bucket's chain)
    struct Node {
        Course course;
        int key;
        Node* next;

        // default constructor makes an empty, unused bucket
        Node() {
            key = UNUSED;
            next = nullptr;
        }

        // initialize with a course and its key
        Node(const Course& aCourse, int aKey) {
            course = aCourse;
            key = aKey;
            next = nullptr;
        }
    };

    vector<Node> buckets;

    unsigned int hash(const string& courseNumber) const;

public:
    HashTable();
    ~HashTable();

    // The table owns memory that was created with new, so copying it
    // would free the same nodes twice. Copying is turned off on purpose.
    HashTable(const HashTable&) = delete;
    HashTable& operator=(const HashTable&) = delete;

    void Clear();
    void Swap(HashTable& other);
    void Insert(const Course& course);
    Course Search(const string& courseNumber) const;
    vector<Course> GetAllCourses() const;
};

/**
 * Default constructor
 */
HashTable::HashTable() {
    // every bucket starts out empty (key = UNUSED, next = nullptr)
    buckets.resize(TABLE_SIZE);
}

/**
 * Destructor
 */
HashTable::~HashTable() {
    // free every chained node so nothing is leaked
    Clear();
}

/**
 * Turn a course number into a bucket number. Course numbers have letters
 * and digits, so add up the character codes of the whole string and then
 * use modulo to land on a valid bucket.
 */
unsigned int HashTable::hash(const string& courseNumber) const {
    unsigned int sum = 0;
    for (unsigned int i = 0; i < courseNumber.length(); ++i) {
        sum = sum + static_cast<unsigned char>(courseNumber[i]);
    }
    return sum % TABLE_SIZE;
}

/**
 * Empty the table: free every chained node and mark every bucket unused.
 * Used before a load (so loading twice never duplicates courses) and after
 * a failed load.
 */
void HashTable::Clear() {
    for (unsigned int i = 0; i < buckets.size(); ++i) {
        // the first node lives inside the vector, so only the chained
        // nodes after it were created with new
        Node* current = buckets[i].next;
        while (current != nullptr) {
            Node* temp = current;
            current = current->next;
            delete temp;
        }
        buckets[i] = Node();
    }
}

/**
 * Trade everything with another table. Loading builds a brand new table
 * first and only swaps it in when the whole file is good, so a bad file
 * never destroys the courses that were already loaded.
 */
void HashTable::Swap(HashTable& other) {
    // the chained nodes are owned through the pointers inside the buckets,
    // so swapping the two vectors hands each table the other's nodes
    buckets.swap(other.buckets);
}

/**
 * Store a course in its bucket, or at the end of that bucket's chain if
 * the bucket is already taken.
 */
void HashTable::Insert(const Course& course) {
    int key = static_cast<int>(hash(course.courseNumber));

    // point at the bucket itself (not a copy) so changes are kept
    Node* node = &buckets[key];

    if (node->key == UNUSED) {
        // empty bucket, store the course right in the bucket
        node->course = course;
        node->key = key;
    } else {
        // collision, walk to the end of the chain and add a new node
        while (node->next != nullptr) {
            node = node->next;
        }
        node->next = new Node(course, key);
    }
}

/**
 * Look up one course by its course number.
 *
 * @return the course, or an empty course (blank courseNumber) if not found
 */
Course HashTable::Search(const string& courseNumber) const {
    int key = static_cast<int>(hash(courseNumber));
    const Node* node = &buckets[key];

    // nothing was ever stored in this bucket
    if (node->key == UNUSED) {
        return Course();
    }

    // check the bucket, then every node chained after it
    while (node != nullptr) {
        if (node->course.courseNumber == courseNumber) {
            return node->course;
        }
        node = node->next;
    }

    return Course();
}

/**
 * Collect every course in the table into one vector. The buckets are in
 * no useful order, so the caller sorts the result.
 */
vector<Course> HashTable::GetAllCourses() const {
    vector<Course> allCourses;

    for (unsigned int i = 0; i < buckets.size(); ++i) {
        if (buckets[i].key != UNUSED) {
            const Node* node = &buckets[i];
            while (node != nullptr) {
                allCourses.push_back(node->course);
                node = node->next;
            }
        }
    }

    return allCourses;
}

//============================================================================
// Helper functions
//============================================================================

/**
 * Remove spaces, tabs and line endings (like the \r in a Windows file)
 * from both ends of a string
 */
string trim(const string& text) {
    size_t first = text.find_first_not_of(" \t\r\n");
    if (first == string::npos) {
        return "";
    }
    size_t last = text.find_last_not_of(" \t\r\n");
    return text.substr(first, last - first + 1);
}

/**
 * Convert a string to upper case so csci400 finds CSCI400
 */
string toUpper(string text) {
    for (unsigned int i = 0; i < text.length(); ++i) {
        text[i] = static_cast<char>(toupper(static_cast<unsigned char>(text[i])));
    }
    return text;
}

/**
 * Split one line on commas and trim every token
 */
vector<string> splitLine(const string& line) {
    vector<string> tokens;
    string token;

    for (unsigned int i = 0; i < line.length(); ++i) {
        if (line[i] == ',') {
            tokens.push_back(trim(token));
            token = "";
        } else {
            token += line[i];
        }
    }
    tokens.push_back(trim(token));    // the last token has no comma after it

    return tokens;
}

/**
 * Build one Course object from tokens that were already split and checked.
 * Blank tokens after the name (like the trailing commas in the data file)
 * are not prerequisites, so they are skipped.
 */
Course parseLineIntoCourse(const vector<string>& tokens) {
    Course newCourse;
    newCourse.courseNumber = toUpper(tokens[0]);
    newCourse.courseName = tokens[1];

    for (unsigned int i = 2; i < tokens.size(); ++i) {
        if (!tokens[i].empty()) {
            newCourse.prerequisites.push_back(toUpper(tokens[i]));
        }
    }

    return newCourse;
}

//============================================================================
// Sorting
//============================================================================

/**
 * Split a range of courses into a low part and a high part around a pivot
 * (the middle course number)
 *
 * @param courses the vector being sorted
 * @param begin first index of the range
 * @param end last index of the range
 * @return the index where the low part ends
 */
int partition(vector<Course>& courses, int begin, int end) {
    int low = begin;
    int high = end;

    // the middle course number is the pivot
    string pivot = courses[begin + (end - begin) / 2].courseNumber;

    bool done = false;
    while (!done) {
        // move low right while its course number comes before the pivot
        while (courses[low].courseNumber < pivot) {
            ++low;
        }

        // move high left while the pivot comes before its course number
        while (pivot < courses[high].courseNumber) {
            --high;
        }

        if (low >= high) {
            // the two sides met, so this range is split
            done = true;
        } else {
            // both are on the wrong side, so swap them and keep going
            swap(courses[low], courses[high]);
            ++low;
            --high;
        }
    }

    return high;
}

/**
 * Sort courses by courseNumber with quick sort
 * Average performance: O(n log(n))
 * Worst case performance: O(n^2)
 */
void quickSort(vector<Course>& courses, int begin, int end) {
    // zero or one course is already sorted
    if (begin >= end) {
        return;
    }

    int mid = partition(courses, begin, end);
    quickSort(courses, begin, mid);
    quickSort(courses, mid + 1, end);
}

//============================================================================
// Loading, printing and the menu
//============================================================================

/**
 * Follow a course's prerequisites (and their prerequisites, and so on) to
 * see if the chain ever comes back to a course already on the path. A chain
 * like CSCI100 needs CSCI101 and CSCI101 needs CSCI100 could never be taken.
 *
 * @param path the courses between the starting course and this one
 * @param finished courses already checked and known to be safe
 * @return true if a circular prerequisite chain was found
 */
bool hasCircularPrerequisites(const HashTable& courses, const string& courseNumber,
                              vector<string>& path, vector<string>& finished) {
    // this course is already on the path, so the chain came back around
    if (find(path.begin(), path.end(), courseNumber) != path.end()) {
        return true;
    }

    // this course was already checked, so there is no need to check it again
    if (find(finished.begin(), finished.end(), courseNumber) != finished.end()) {
        return false;
    }

    path.push_back(courseNumber);

    Course course = courses.Search(courseNumber);
    for (unsigned int i = 0; i < course.prerequisites.size(); ++i) {
        if (hasCircularPrerequisites(courses, course.prerequisites[i], path, finished)) {
            return true;
        }
    }

    path.pop_back();
    finished.push_back(courseNumber);
    return false;
}

/**
 * Open the file, read and check every line, create a Course for each valid
 * line and store it in a new hash table, then confirm every prerequisite is
 * a course in the file.
 *
 * Policy: a bad line is reported and skipped, but if any prerequisite has no
 * matching course (or the prerequisites go in a circle) the whole load is
 * stopped. The table that is already loaded is only replaced when the whole
 * file loads, so a bad file never wipes out good data.
 *
 * @return true only if the courses were loaded
 */
bool loadCourses(const string& fileName, HashTable& courses) {
    if (fileName.empty()) {
        cout << "Error: no file name was entered." << endl;
        return false;
    }

    ifstream file(fileName.c_str());
    if (!file.is_open()) {
        cout << "Error: could not open file " << fileName << endl;
        return false;
    }

    // pass 1: read every line that has text
    vector<string> fileLines;
    string currentLine;
    bool firstLine = true;
    while (getline(file, currentLine)) {
        // files saved from Excel can start with an invisible marker (a BOM)
        // that would stick to the first course number, so remove it
        if (firstLine && currentLine.compare(0, 3, "\xEF\xBB\xBF") == 0) {
            currentLine = currentLine.substr(3);
        }
        firstLine = false;

        if (!trim(currentLine).empty()) {
            fileLines.push_back(currentLine);
        }
    }
    file.close();

    if (fileLines.empty()) {
        cout << "Error: the file has no course data." << endl;
        return false;
    }

    // pass 2: check each line, create the Course and store it
    HashTable newCourses;
    vector<Course> loadedCourses;
    for (unsigned int i = 0; i < fileLines.size(); ++i) {
        vector<string> tokens = splitLine(fileLines[i]);

        // every line needs a course number and a course name
        if (tokens.size() < 2 || tokens[0].empty() || tokens[1].empty()) {
            cout << "Error: line is missing a course number or a course name -> "
                 << trim(fileLines[i]) << endl;
            continue;
        }

        Course newCourse = parseLineIntoCourse(tokens);

        // a course number can only appear once
        if (!newCourses.Search(newCourse.courseNumber).courseNumber.empty()) {
            cout << "Error: course " << newCourse.courseNumber
                 << " is listed more than once, so the repeat was skipped" << endl;
            continue;
        }

        newCourses.Insert(newCourse);
        loadedCourses.push_back(newCourse);
    }

    // every line was bad, so there is nothing to load
    if (loadedCourses.empty()) {
        cout << "Error: the file has no valid course lines." << endl;
        return false;
    }

    // pass 3: every prerequisite must be a course in the file. This waits
    // until every course is stored so a prerequisite listed further down
    // the file is still found.
    bool hasError = false;
    for (unsigned int i = 0; i < loadedCourses.size(); ++i) {
        for (unsigned int j = 0; j < loadedCourses[i].prerequisites.size(); ++j) {
            const string& prereq = loadedCourses[i].prerequisites[j];

            if (newCourses.Search(prereq).courseNumber.empty()) {
                cout << "Error: " << loadedCourses[i].courseNumber
                     << " lists prerequisite " << prereq
                     << " which has no matching course" << endl;
                hasError = true;
            }
        }
    }

    // pass 4: no course can depend on itself, even through other courses.
    // Skipped when a prerequisite is missing, because the chain is broken.
    if (!hasError) {
        vector<string> finished;
        for (unsigned int i = 0; i < loadedCourses.size(); ++i) {
            vector<string> path;
            if (hasCircularPrerequisites(newCourses, loadedCourses[i].courseNumber,
                                         path, finished)) {
                cout << "Error: " << loadedCourses[i].courseNumber
                     << " is part of, or depends on, a circular prerequisite chain" << endl;
                hasError = true;
            }
        }
    }

    // one bad prerequisite stops the whole load (newCourses is thrown away
    // when this function ends, and the courses already loaded stay as they were)
    if (hasError) {
        cout << "No courses were loaded. Fix the errors above and load the file again."
             << endl;
        return false;
    }

    // everything checked out, so swap the new table in
    courses.Swap(newCourses);
    cout << loadedCourses.size() << " courses loaded." << endl;
    return true;
}

/**
 * Print every course in alphanumeric order by course number
 */
void printAllCourses(const HashTable& courses) {
    // the hash table has no order, so collect the courses and sort them
    vector<Course> allCourses = courses.GetAllCourses();
    if (!allCourses.empty()) {
        quickSort(allCourses, 0, static_cast<int>(allCourses.size()) - 1);
    }

    cout << "Here is a sample schedule:" << endl << endl;
    for (unsigned int i = 0; i < allCourses.size(); ++i) {
        cout << allCourses[i].courseNumber << ", " << allCourses[i].courseName << endl;
    }
}

/**
 * Print the title and the prerequisites of one course
 */
void printCourse(const HashTable& courses, const string& courseNumber) {
    Course course = courses.Search(courseNumber);

    if (course.courseNumber.empty()) {
        cout << "Course " << courseNumber << " not found." << endl;
        return;
    }

    cout << course.courseNumber << ", " << course.courseName << endl;

    if (course.prerequisites.empty()) {
        cout << "Prerequisites: None" << endl;
        return;
    }

    if (SHOW_PREREQUISITE_TITLES) {
        // one prerequisite per line with its title (every prerequisite was
        // checked when the file was loaded, so each one can be found)
        cout << "Prerequisites:" << endl;
        for (unsigned int i = 0; i < course.prerequisites.size(); ++i) {
            Course prereq = courses.Search(course.prerequisites[i]);
            cout << "    " << prereq.courseNumber << ", " << prereq.courseName << endl;
        }
        return;
    }

    // matches the sample output: all the prerequisite numbers on one line
    cout << "Prerequisites: ";
    for (unsigned int i = 0; i < course.prerequisites.size(); ++i) {
        if (i > 0) {
            cout << ", ";
        }
        cout << course.prerequisites[i];
    }
    cout << endl;
}

/**
 * Show the menu
 */
void printMenu() {
    cout << endl;
    cout << "   1. Load Data Structure." << endl;
    cout << "   2. Print Course List." << endl;
    cout << "   3. Print Course." << endl;
    cout << "   9. Exit" << endl;
    cout << endl;
    cout << "What would you like to do? ";
}

/**
 * The one and only main() method
 */
int main() {
    HashTable courses;
    bool dataLoaded = false;
    string choice;

    cout << "Welcome to the course planner." << endl;

    while (choice != "9") {
        printMenu();

        // read the whole line so letters or blank input cannot break the menu
        if (!getline(cin, choice)) {
            break;    // input ended, so stop instead of looping forever
        }
        choice = trim(choice);

        if (choice == "1") {
            cout << "Enter the file name: ";
            string fileName;
            getline(cin, fileName);

            if (loadCourses(trim(fileName), courses)) {
                dataLoaded = true;
            } else if (dataLoaded) {
                // a failed load leaves the earlier courses untouched
                cout << "The courses you loaded earlier are still available." << endl;
            }

        } else if (choice == "2") {
            if (!dataLoaded) {
                cout << "Please load the data first (option 1)." << endl;
            } else {
                printAllCourses(courses);
            }

        } else if (choice == "3") {
            if (!dataLoaded) {
                cout << "Please load the data first (option 1)." << endl;
            } else {
                cout << "What course do you want to know about? ";
                string courseNumber;
                getline(cin, courseNumber);
                printCourse(courses, toUpper(trim(courseNumber)));
            }

        } else if (choice == "9") {
            cout << "Thank you for using the course planner!" << endl;

        } else if (choice.empty()) {
            cout << "Please enter a menu option (1, 2, 3 or 9)." << endl;

        } else {
            cout << choice << " is not a valid option." << endl;
        }
    }

    return 0;
}
