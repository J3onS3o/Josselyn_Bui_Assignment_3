/******************************************************************************
 * Program Name: EECS 348 Assignment 3 - CEO Email Priority Queue
 *
 * Description:
 *     C++ program that prioritizes emails for a busy CEO. Emails are stored in
 *     a custom, list-based (vector) MaxHeap that is built from scratch, with
 *     no pre-existing heap modules. Emails are prioritized first by sender
 *     category (Boss > Subordinate > Peer > ImportantPerson > OtherPerson) and
 *     then by date, where the newest email is read first within a category.
 *
 * Inputs:
 *     The name of a text file passed as a command-line argument:
 *         ./program input.txt
 *     Each line of the file is one command:
 *         EMAIL <sender category>,<subject line>,<date MM-DD-YYYY>
 *         NEXT   (display the highest priority unread email)
 *         READ   (remove the highest priority unread email)
 *         COUNT  (display the number of unread emails)
 *
 * Outputs:
 *     COUNT prints:  There are N emails to read.
 *     NEXT prints:   Next email: / Sender: / Subject: / Date:
 *     NEXT on an empty queue prints: No emails to read.
 *     Warnings (bad sender category, unknown command) and file errors go to
 *     standard error so they never mix with the normal output.
 *
 * Collaborators:
 *     - OpenAI ChatGPT (GPT-5.6 Luna): original program that this file is
 *       based on (selected over the Gemini version in the GenAI analysis).
 *     - Google Gemini: second GenAI program, used only for the comparison.
 *
 * Author: Josselyn T. Bui
 * Creation Date: 10/01/2026
 * Revision Date: 10/02/2026
 *
 * Revisions:
 *     1. Moved trim() from a free function into the StringUtils class and
 *        moved the argument handling into EmailProgram so main() only creates
 *        one object and calls run().
 *     2. Added Email::isValidCategory() and reject emails with an unknown
 *        sender category with a warning instead of treating them as lowest
 *        priority.
 *     3. Print a warning for unknown commands instead of ignoring them.
 *     4. Used std::move in insert() and removeMax() to avoid copying strings.
 *     5. Removed unused includes (<algorithm>, <sstream>).
 *     6. Marked helper methods const and removed "using namespace std".
 *     7. Completed the prologue and added authorship labels to every block.
 *
 * Authorship key (each block below carries one of these labels):
 *     [CHATGPT]       Written by ChatGPT, unchanged.
 *     [CHATGPT+EDIT]  Written by ChatGPT, then edited by Josselyn T. Bui.
 ******************************************************************************/

#include <iostream>   // Provides std::cout, std::cerr, and std::endl for output.
#include <fstream>    // Provides std::ifstream for reading the input file.
#include <string>     // Provides the std::string class.
#include <vector>     // Provides std::vector, used as the heap's list.
#include <utility>    // Provides std::swap and std::move.
#include <cstddef>    // Provides std::size_t.

// how to run this in terminal?
// 1. Compile the program: g++ -o email_program main.cpp
// 2. Run the program: ./email_program <input_file>

/******************************************************************************
 * Class: StringUtils
 * Authorship: [CHATGPT+EDIT] trim() was a free function in the ChatGPT code
 *             and was moved into this class (Revision 1).
 *
 * Description:
 *     Holds string helper methods as static members so no free functions
 *     are needed.
 ******************************************************************************/
class StringUtils
{
public:
    /**************************************************************************
     * Function: trim
     *
     * Description:
     *     Removes whitespace from the beginning and end of a string.
     *
     * Parameters:
     *     str - The string that should be trimmed.
     *
     * Returns:
     *     A copy of the string with surrounding whitespace removed.
     **************************************************************************/
    static std::string trim(const std::string& str)
    {
        // Find the first character that is not whitespace.
        std::size_t first = str.find_first_not_of(" \t\r\n");

        // If the string contains only whitespace, return an empty string.
        if (first == std::string::npos)
        {
            // Return the empty string.
            return "";
        }

        // Find the last character that is not whitespace.
        std::size_t last = str.find_last_not_of(" \t\r\n");

        // Return the substring between the first and last non-whitespace characters.
        return str.substr(first, last - first + 1);
    }
};

/******************************************************************************
 * Class: Email
 * Authorship: [CHATGPT+EDIT] Class and methods are from ChatGPT. The
 *             isValidCategory() method is new Josselyn Revision -- (Revision 2), and the
 *             helper methods were marked const (Revision 6).
 *
 * Description:
 *     Represents one email. The class stores the sender category, subject,
 *     and date and provides a priority comparison between emails.
 ******************************************************************************/
class Email
{
private:
    // Stores the sender category of the email.
    std::string sender;

    // Stores the subject line of the email.
    std::string subject;

    // Stores the date of the email in MM-DD-YYYY format.
    std::string date;

    // Stores the numerical priority of the sender category.
    int senderPriority;

    /**************************************************************************
     * Function: calculateSenderPriority
     * Authorship: [CHATGPT+EDIT] Marked const (Revision 6).
     *
     * Description:
     *     Converts a sender category into its priority number.
     *
     * Returns:
     *     A larger number means higher priority.
     **************************************************************************/
    int calculateSenderPriority(const std::string& category) const
    {
        // Boss has the highest sender priority.
        if (category == "Boss")
        {
            // Return the highest priority number.
            return 5;
        }

        // Subordinate has the second-highest sender priority.
        if (category == "Subordinate")
        {
            // Return the second-highest priority number.
            return 4;
        }

        // Peer has the third-highest sender priority.
        if (category == "Peer")
        {
            // Return the middle priority number.
            return 3;
        }

        // ImportantPerson has the fourth-highest sender priority.
        if (category == "ImportantPerson")
        {
            // Return the second-lowest priority number.
            return 2;
        }

        // OtherPerson (and anything else) has the lowest sender priority.
        return 1;
    }

    /**************************************************************************
     * Function: dateValue
     * Authorship: [CHATGPT+EDIT] Converted to the version with a temporary
     *             variable and marked const; logic is unchanged.
     *
     * Description:
     *     Converts MM-DD-YYYY into a number that can be compared. The input
     *     format is assumed to be valid, as stated in the assignment.
     *
     * Returns:
     *     A numeric representation of the date (YYYYMMDD).
     **************************************************************************/
    long long dateValue() const
    {
        // Extract the month from the first two characters of the date.
        int month = std::stoi(date.substr(0, 2));

        // Extract the day from characters 3 and 4 of the date.
        int day = std::stoi(date.substr(3, 2));

        // Extract the year from the last four characters of the date.
        int year = std::stoi(date.substr(6, 4));

        // Convert the date to YYYYMMDD so chronological comparison works.
        long long value = year * 10000LL + month * 100LL + day;

        // Return the converted date.
        return value;
    }

public:
    /**************************************************************************
     * Function: isValidCategory
     * Authorship: Josselyn Revision -- New method (Revision 2).
     *
     * Description:
     *     Checks whether a string is exactly one of the five allowed sender
     *     categories. It is static so it can be called before an Email exists.
     *
     * Parameters:
     *     category - The sender category text to check.
     *
     * Returns:
     *     true if the category is valid, false otherwise.
     **************************************************************************/
    static bool isValidCategory(const std::string& category)
    {
        // Return true only when the text exactly matches one of the five categories.
        return category == "Boss" ||
               category == "Subordinate" ||
               category == "Peer" ||
               category == "ImportantPerson" ||
               category == "OtherPerson";
    }

    /**************************************************************************
     * Constructor: Email
     * Authorship: [CHATGPT]
     *
     * Description:
     *     Creates an Email object using the supplied information.
     **************************************************************************/
    Email(const std::string& senderInput,
          const std::string& subjectInput,
          const std::string& dateInput)
    {
        // Store the sender category.
        sender = senderInput;

        // Store the subject.
        subject = subjectInput;

        // Store the date.
        date = dateInput;

        // Calculate and store the sender's priority.
        senderPriority = calculateSenderPriority(sender);
    }

    /**************************************************************************
     * Function: higherPriorityThan
     * Authorship: [CHATGPT]
     *
     * Description:
     *     Determines whether this email has a higher priority than another
     *     email.
     *
     * Parameters:
     *     other - The email being compared against.
     *
     * Returns:
     *     true if this email should appear before the other email.
     **************************************************************************/
    bool higherPriorityThan(const Email& other) const
    {
        // Compare sender categories first.
        if (senderPriority != other.senderPriority)
        {
            // The larger sender priority wins.
            return senderPriority > other.senderPriority;
        }

        // If sender categories are equal, compare the dates.
        if (dateValue() != other.dateValue())
        {
            // The newer date has higher priority.
            return dateValue() > other.dateValue();
        }

        // If both category and date are equal, treat them as equal priority.
        return false;
    }

    /**************************************************************************
     * Function: getSender
     * Authorship: [CHATGPT]
     *
     * Description:
     *     Returns the sender category.
     **************************************************************************/
    std::string getSender() const
    {
        // Return the sender category.
        return sender;
    }

    /**************************************************************************
     * Function: getSubject
     * Authorship: [CHATGPT]
     *
     * Description:
     *     Returns the email subject.
     **************************************************************************/
    std::string getSubject() const
    {
        // Return the subject.
        return subject;
    }

    /**************************************************************************
     * Function: getDate
     * Authorship: [CHATGPT]
     *
     * Description:
     *     Returns the email date.
     **************************************************************************/
    std::string getDate() const
    {
        // Return the date.
        return date;
    }
};

/******************************************************************************
 * Class: MaxHeap
 * Authorship: [CHATGPT+EDIT] Class is from ChatGPT. Helper methods were
 *             marked const (Revision 6), and insert() and removeMax() now use
 *             std::move (Revision 4).
 *
 * Description:
 *     Implements a MaxHeap from scratch using a vector as an array.
 *     The highest-priority email is always stored at index 0.
 ******************************************************************************/
class MaxHeap
{
private:
    // Stores all emails in the heap's underlying array.
    std::vector<Email> heap;

    /**************************************************************************
     * Function: parent
     * Authorship: [CHATGPT+EDIT] Marked const (Revision 6).
     *
     * Description:
     *     Calculates the parent index of a node.
     **************************************************************************/
    int parent(int index) const
    {
        // Return the parent index.
        return (index - 1) / 2;
    }

    /**************************************************************************
     * Function: leftChild
     * Authorship: [CHATGPT+EDIT] Marked const (Revision 6).
     *
     * Description:
     *     Calculates the left child index of a node.
     **************************************************************************/
    int leftChild(int index) const
    {
        // Return the left child index.
        return 2 * index + 1;
    }

    /**************************************************************************
     * Function: rightChild
     * Authorship: [CHATGPT+EDIT] Marked const (Revision 6).
     *
     * Description:
     *     Calculates the right child index of a node.
     **************************************************************************/
    int rightChild(int index) const
    {
        // Return the right child index.
        return 2 * index + 2;
    }

    /**************************************************************************
     * Function: heapifyUp
     * Authorship: [CHATGPT]
     *
     * Description:
     *     Moves a newly inserted email upward until the MaxHeap property
     *     is restored.
     **************************************************************************/
    void heapifyUp(int index)
    {
        // Continue while the current element is not the root.
        while (index > 0)
        {
            // Find the parent of the current element.
            int parentIndex = parent(index);

            // Stop if the parent already has higher or equal priority.
            if (!heap[index].higherPriorityThan(heap[parentIndex]))
            {
                // The heap property holds, so leave the loop.
                break;
            }

            // Swap the current email with its parent.
            std::swap(heap[index], heap[parentIndex]);

            // Continue checking from the parent's old position.
            index = parentIndex;
        }
    }

    /**************************************************************************
     * Function: heapifyDown
     * Authorship: [CHATGPT]
     *
     * Description:
     *     Moves the root email downward after removing it so that the
     *     MaxHeap property is restored.
     **************************************************************************/
    void heapifyDown(int index)
    {
        // Continue checking the current node.
        while (true)
        {
            // Calculate the left child index.
            int left = leftChild(index);

            // Calculate the right child index.
            int right = rightChild(index);

            // Assume the current node has the highest priority.
            int highest = index;

            // Check whether the left child has higher priority.
            if (left < static_cast<int>(heap.size()) &&
                heap[left].higherPriorityThan(heap[highest]))
            {
                // The left child becomes the highest-priority node.
                highest = left;
            }

            // Check whether the right child has higher priority.
            if (right < static_cast<int>(heap.size()) &&
                heap[right].higherPriorityThan(heap[highest]))
            {
                // The right child becomes the highest-priority node.
                highest = right;
            }

            // If the current node is still highest, the heap is valid.
            if (highest == index)
            {
                // The heap property holds, so leave the loop.
                break;
            }

            // Swap the current node with the higher-priority child.
            std::swap(heap[index], heap[highest]);

            // Continue heapifying from the child's old position.
            index = highest;
        }
    }

public:
    /**************************************************************************
     * Function: insert
     * Authorship: [CHATGPT+EDIT] Now takes the email by value and moves it
     *             into the vector (Revision 4).
     *
     * Description:
     *     Adds an email to the MaxHeap.
     **************************************************************************/
    void insert(Email email)
    {
        // Move the new email to the end of the array (no string copies).
        heap.push_back(std::move(email));

        // Move the new email upward into its correct position.
        heapifyUp(static_cast<int>(heap.size()) - 1);
    }

    /**************************************************************************
     * Function: peek
     * Authorship: [CHATGPT]
     *
     * Description:
     *     Returns the highest-priority email without removing it. The caller
     *     must check that the heap is not empty first.
     *
     * Returns:
     *     A reference to the highest-priority email.
     **************************************************************************/
    const Email& peek() const
    {
        // Return the root of the MaxHeap.
        return heap[0];
    }

    /**************************************************************************
     * Function: removeMax
     * Authorship: [CHATGPT+EDIT] Uses std::move instead of copying the last
     *             email into the root (Revision 4).
     *
     * Description:
     *     Removes the highest-priority email from the MaxHeap.
     **************************************************************************/
    void removeMax()
    {
        // Do nothing if the heap is empty.
        if (heap.empty())
        {
            // Nothing to remove.
            return;
        }

        // If there is only one email, simply remove it.
        if (heap.size() == 1)
        {
            // Remove the only element.
            heap.pop_back();

            // Nothing left to reorder.
            return;
        }

        // Move the last email to the root position (no string copies).
        heap[0] = std::move(heap.back());

        // Remove the moved-from last element.
        heap.pop_back();

        // Restore the MaxHeap property.
        heapifyDown(0);
    }

    /**************************************************************************
     * Function: empty
     * Authorship: [CHATGPT]
     *
     * Description:
     *     Determines whether the heap contains any emails.
     **************************************************************************/
    bool empty() const
    {
        // Return true when there are no emails.
        return heap.empty();
    }

    /**************************************************************************
     * Function: size
     * Authorship: [CHATGPT]
     *
     * Description:
     *     Returns the number of emails in the heap.
     **************************************************************************/
    int size() const
    {
        // Return the number of emails.
        return static_cast<int>(heap.size());
    }
};

/******************************************************************************
 * Class: EmailPriorityQueue
 * Authorship: [CHATGPT+EDIT] Class is from ChatGPT. processEmail() now
 *             validates the sender category (Revision 2) and processCommand()
 *             warns about unknown commands (Revision 3). Both use
 *             StringUtils::trim (Revision 1).
 *
 * Description:
 *     Provides the commands needed to manage emails using the MaxHeap.
 ******************************************************************************/
class EmailPriorityQueue
{
private:
    // Stores all unread emails in the custom MaxHeap.
    MaxHeap emailHeap;

    /**************************************************************************
     * Function: processEmail
     * Authorship: [CHATGPT+EDIT] Category validation block is new Josselyn Revision --.
     *
     * Description:
     *     Parses an EMAIL command and inserts the resulting email.
     **************************************************************************/
    void processEmail(const std::string& line)
    {
        // Remove the "EMAIL " portion from the beginning of the command.
        std::string data = StringUtils::trim(line.substr(6));

        // Find the first comma separating sender and subject.
        std::size_t firstComma = data.find(',');

        // Find the second comma separating subject and date.
        std::size_t secondComma = data.find(',', firstComma + 1);

        // Stop if the expected commas are missing.
        if (firstComma == std::string::npos ||
            secondComma == std::string::npos)
        {
            // Skip the malformed line.
            return;
        }

        // Extract and trim the sender category.
        std::string sender = StringUtils::trim(data.substr(0, firstComma));

        // Extract and trim the subject.
        std::string subject = StringUtils::trim(
            data.substr(firstComma + 1,
                        secondComma - firstComma - 1)
        );

        // Extract and trim the date.
        std::string date = StringUtils::trim(data.substr(secondComma + 1));

        // Josselyn Revision -- Reject emails whose sender category is not one of the five.
        if (!Email::isValidCategory(sender))
        {
            // Josselyn Revision -- Report the bad category on standard error.
            std::cerr << "Warning: skipping email with unknown sender category: "
                      << sender << std::endl;

            // Josselyn Revision -- Skip the email so it is never inserted.
            return;
        }

        // Create an Email object using the parsed fields.
        Email email(sender, subject, date);

        // Move the email into the priority queue.
        emailHeap.insert(std::move(email));
    }

public:
    /**************************************************************************
     * Function: processCommand
     * Authorship: [CHATGPT+EDIT] The final else branch is new Josselyn Revision --.
     *
     * Description:
     *     Determines which command was entered and performs that command.
     **************************************************************************/
    void processCommand(const std::string& inputLine)
    {
        // Trim whitespace from the input line.
        std::string line = StringUtils::trim(inputLine);

        // Ignore blank lines.
        if (line.empty())
        {
            // Nothing to do for an empty line.
            return;
        }

        // Process an EMAIL command.
        if (line.rfind("EMAIL ", 0) == 0)
        {
            // Parse the line and insert the email.
            processEmail(line);
        }
        // Process a NEXT command.
        else if (line == "NEXT")
        {
            // Display the highest-priority email.
            next();
        }
        // Process a READ command.
        else if (line == "READ")
        {
            // Remove the highest-priority email.
            read();
        }
        // Process a COUNT command.
        else if (line == "COUNT")
        {
            // Display the number of unread emails.
            count();
        }
        // Josselyn Revision -- Anything else is an unknown command.
        else
        {
            // Josselyn Revision -- Warn on standard error so the output stays clean.
            std::cerr << "Warning: unknown command skipped: " << line << std::endl;
        }
    }

    /**************************************************************************
     * Function: next
     * Authorship: [CHATGPT]
     *
     * Description:
     *     Displays the highest-priority unread email without removing it.
     **************************************************************************/
    void next()
    {
        // Check whether the queue is empty.
        if (emailHeap.empty())
        {
            // Print the required empty-queue message.
            std::cout << "No emails to read." << std::endl;

            // Stop because there is no email to display.
            return;
        }

        // Get the highest-priority email without removing it.
        const Email& email = emailHeap.peek();

        // Print the required NEXT output.
        std::cout << "Next email:" << std::endl;

        // Print the sender category.
        std::cout << "Sender: " << email.getSender() << std::endl;

        // Print the subject.
        std::cout << "Subject: " << email.getSubject() << std::endl;

        // Print the date.
        std::cout << "Date: " << email.getDate() << std::endl;
    }

    /**************************************************************************
     * Function: read
     * Authorship: [CHATGPT]
     *
     * Description:
     *     Removes the highest-priority email without displaying it.
     **************************************************************************/
    void read()
    {
        // Remove the highest-priority email.
        emailHeap.removeMax();
    }

    /**************************************************************************
     * Function: count
     * Authorship: [CHATGPT]
     *
     * Description:
     *     Displays the number of unread emails.
     **************************************************************************/
    void count()
    {
        // Print the number of unread emails using the required format.
        std::cout << "There are " << emailHeap.size()
                  << " emails to read." << std::endl;
    }
};

/******************************************************************************
 * Class: EmailProgram
 * Authorship: [CHATGPT+EDIT] File reading loop is from ChatGPT. The
 *             constructor now receives argc and argv and run() performs the
 *             usage check that used to be in main() Josselyn Revision -- (Revision 1).
 *
 * Description:
 *     Controls the program's argument checking, file handling, and command
 *     processing.
 ******************************************************************************/
class EmailProgram
{
private:
    // Josselyn Revision -- Stores how many command-line arguments were supplied.
    int argumentCount;

    // Stores the name of the input file.
    std::string filename;

    // Stores the email priority queue.
    EmailPriorityQueue queue;

public:
    /**************************************************************************
     * Constructor: EmailProgram
     * Authorship: [CHATGPT+EDIT] Takes argc and argv instead of a filename.
     *
     * Description:
     *     Creates the program using the command-line arguments.
     **************************************************************************/
    EmailProgram(int argc, char* argv[])
    {
        // Josselyn Revision -- Remember how many arguments were supplied.
        argumentCount = argc;

        // Josselyn Revision -- Store the filename only when the argument exists.
        if (argc >= 2)
        {
            // Josselyn Revision -- The input filename is the first argument after the program name.
            filename = argv[1];
        }
    }

    /**************************************************************************
     * Function: run
     * Authorship: [CHATGPT+EDIT] Usage check at the top is new Josselyn Revision --.
     *
     * Description:
     *     Checks the arguments, opens the input file, and processes every line.
     *
     * Returns:
     *     0 if the file was successfully processed.
     *     1 if the filename is missing or the file could not be opened.
     **************************************************************************/
    int run()
    {
        // Josselyn Revision -- Check that an input filename was supplied.
        if (argumentCount < 2)
        {
            // Josselyn Revision -- Print the required usage message.
            std::cout << "Usage: ./program <input_file>" << std::endl;

            // Josselyn Revision -- Return a failure status because there is no input file.
            return 1;
        }

        // Open the input file.
        std::ifstream inputFile(filename);

        // Check whether the file opened successfully.
        if (!inputFile.is_open())
        {
            // Print an error containing the filename.
            std::cerr << "Error: could not open file " << filename << std::endl;

            // Return a failure status.
            return 1;
        }

        // Store one line from the input file at a time.
        std::string line;

        // Read every line, including the final line without a newline.
        while (std::getline(inputFile, line))
        {
            // Remove a trailing carriage return from Windows CRLF files.
            if (!line.empty() && line.back() == '\r')
            {
                // Drop the carriage return character.
                line.pop_back();
            }

            // Process the current command.
            queue.processCommand(line);
        }

        // Close the input file.
        inputFile.close();

        // Return a successful status.
        return 0;
    }
};

/******************************************************************************
 * Function: main
 * Authorship: [CHATGPT+EDIT] Argument check moved into EmailProgram so main
 *             only creates one object and calls run() (Revision 1).
 *
 * Description:
 *     Creates the program object and starts execution.
 *
 * Parameters:
 *     argc - Number of command-line arguments.
 *     argv - Array containing the command-line arguments.
 *
 * Returns:
 *     Program exit status.
 ******************************************************************************/
int main(int argc, char* argv[])
{
    // Create the main program object using the command-line arguments.
    EmailProgram program(argc, argv);

    // Run the program and return its status.
    return program.run();
}