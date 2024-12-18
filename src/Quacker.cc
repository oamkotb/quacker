#include "Quacker.hh"

// =============================================================================
// Public Methods
// =============================================================================

/**
 * @brief Constructs a Quacker object and attempts to load the database.
 *
 * This constructor initializes the Quacker object and attempts to load
 * the database from the specified file. If the database cannot be loaded,
 * an error message is printed to `std::cerr`, and the program exits with
 * a status code of ERROR_SQL.
 *
 * @param db_filename The name of the database file to load.
 *
 * @note Ensure that the provided `db_filename` points to a valid and
 * accessible database file to prevent the program from terminating.
 */
Quacker::Quacker(const std::string& db_filename) {
  if (pond.loadDatabase(db_filename)) {
    std::cerr << "Database Error: Could Not Open" << db_filename << std::endl;
    exit(ERROR_SQL);
  }
}

/**
 * @brief Destructor for the Quacker class.
 *
 * This destructor clears the console by executing the `clear` system command 
 * and releases the memory allocated for the `_user_id` member variable.
 */
Quacker::~Quacker() {
  std::system("clear");
  if (_user_id) {
    delete _user_id;
  }
}

/**
 * @brief Initiates the main execution flow of the Quacker application.
 *
 * This method serves as the entry point for running the Quacker application.
 * It begins by calling the `startPage()` method, which likely handles
 * the initial setup or user interface for the application.
 */
void Quacker::run() {
  startPage();
}

// =============================================================================
// Private Methods
// =============================================================================

/**
 * @brief Displays the main start page for the Quacker application and prompts user actions.
 *
 * This function continually displays the main start page menu until the user logs in or exits.
 * While `_user_id` remains `nullptr`, the menu provides options to log in, sign up, or exit the program.
 * Each option triggers the corresponding page or action.
 *
 * The menu options include:
 * - **1. Log in**: Opens the login page.
 * - **2. Sign up**: Opens the signup page.
 * - **3. Exit**: Terminates the program.
 */
void Quacker::startPage() {
  std::string error = "";
  while (this->_user_id == nullptr) {
    std::system("clear");

    char select;
    std::cout << QUACKER_BANNER << error << "\n1. Log in\n2. Sign up\n3. Exit\n\nSelection: ";
    std::cin >> select;
    
    if (std::cin.peek() != '\n') select = '0';
    // Consume any trailing '\n' and discard it
    { std::string dummy; std::getline(std::cin, dummy); }
    switch (select) {
      case '1':
        error = "";
        loginPage();
        break;
      case '2':
        error = "";
        signupPage();
        
        break;
      case '3':
        std::system("clear");
        error = "";
        exit(0);
        break;

      default:
        error = "\nInvalid Input Entered [use: 1, 2, 3]\n";
        break;
    }
  }
}

/**
 * @brief Manages user login by validating credentials.
 *
 * This method provides an interface for users to log in by entering their 
 * User ID and password. It verifies the credentials and logs the user into 
 * their account upon successful authentication.
 *
 * @details
 * - Validates the User ID to ensure it is a valid integer.
 * - Securely captures the password input.
 * - Checks the entered credentials against the database and provides feedback 
 *   if authentication fails.
 * - Allows the user to exit the login process by pressing Enter without input.
 */
void Quacker::loginPage() {
  std::string description = "Enter login credentials or press Enter to return.";

  while (true) {
    // Clear the screen and show the login interface
    std::system("clear");
    std::cout << QUACKER_BANNER << "\n" << description << "\n\n--- Log In ---\n" << "\nUser ID: ";

    std::string user_id_str;
    int32_t user_id;
    std::string password;

    // Get the user ID input from the user
    std::getline(std::cin, user_id_str);
    if (user_id_str.empty()) {
      return;
    } else if (isID(user_id_str)) {
      try {
        user_id = std::stoi(user_id_str);
      } catch (const std::out_of_range&) {
        description = "Invalid User ID, ID must be a valid integer.";
        continue;
      }
    } else {
      description = "Invalid User ID, ID must be a valid integer.";
      continue;
    }

    // Ask for the password
    std::cout << "Password: ";
    password = this->getHiddenPassword();
    // std::getline(std::cin, password);

    // Check login credentials
    this->_user_id = pond.checkLogin(user_id, password);

    // If credentials are invalid, prompt the user to try again
    if (_user_id == nullptr) {
      description = "Invalid credentials, please enter a valid 'User ID' and 'Password', or press Enter to return.";
      continue;
    }
    break;
  }
  logged_in = true;
  mainPage();
}

/**
 * @brief Handles user registration by collecting and validating input.
 *
 * This method provides an interface for new users to sign up by entering their
 * name, email, phone number, and password. It validates the input and adds the 
 * user to the database upon successful registration.
 *
 * @details
 * - Ensures the email follows a valid format and the phone number is properly structured.
 * - Safely captures and stores the password.
 * - Attempts to register the user and provides feedback if the process encounters errors.
 * - Automatically logs the user in after successful registration.
 */
void Quacker::signupPage() {
  std::string description = "Enter your details or press Enter to return... ";
  while (true) {
    // Clear the screen and show the sign-up interface
    std::system("clear");
    std::cout << QUACKER_BANNER << "\n" << description << "\n\n--- Sign Up ---\n";

    std::string name, email, phone_str, password;
    long int phone_number;

    // Get and validate the name
    std::cout << "Enter Name: ";
    std::getline(std::cin, name);
    name = trim(name);
    if (name.empty()) return;

    // Get and validate the email
    std::cout << "Enter Email: ";
    std::getline(std::cin, email);
    if (email.empty()) return;
    if (!isValidEmail(email)) {
      description = "Invalid email format, please try again [eg. example@ualberta.ca]";
      continue;
    }

    // Get and validate the phone number
    std::cout << "Enter Phone Number: ";
    std::getline(std::cin, phone_str);
    if (phone_str.empty()) return;
    phone_number = isValidPhoneNumber(phone_str);
    if (phone_number == -1) {
      description = "Invalid phone number format, please try again [eg. 510-827-7791].";
      continue;
    }

    // Get and validate the password
    std::cout << "Enter Password: ";
    password = this->getHiddenPassword();
    // std::getline(std::cin, password);
    if (password.empty()) return;

    // Add user to the database
    int32_t* new_user_id = pond.addUser(name, email, phone_number, password);
    
    // If the user is successfully added, assign the new user ID to _user_id and notify the user
    if (new_user_id != nullptr) {
      this->_user_id = new_user_id;
      std::cout << "Account created! Press Enter to log in... ";
      std::cin.get();
      logged_in = true;
      mainPage();
      break;
    } else {
        description = "Error during signup, please try again.\n";
    }
  }
}

/**
 * @brief Manages the main user interface for logged-in users.
 *
 * This method serves as the central hub for logged-in users, providing access 
 * to features such as viewing and interacting with their feed, searching for 
 * users or posts, creating new posts, and logging out.
 *
 * @details
 * - Displays the user feed and adjusts the number of visible posts based on user selection.
 * - Validates user input to ensure actions correspond to available menu options.
 * - Provides options for replying to or retweeting posts directly from the feed.
 * - Handles logging out by cleaning up the session and redirecting to the start page.
 */
void Quacker::mainPage() {
  std::string error = "";
  int32_t FeedDisplayCount = 5;
  while (logged_in) {
    std::system("clear");
    
    std::string username = pond.getUsername(*(this->_user_id));

    int32_t i = 1;
    char select;
    std::cout << QUACKER_BANNER << "\nWelcome back, " << username 
    << "! (User Id: " << *(this->_user_id) << ")\n\n-------------------------------------------- Your Feed ---------------------------------------------\n";
    std::cout << processFeed(FeedDisplayCount, error, i);
    std::cout << "\n" << error << "\n\n1. See More Of My Feed\n"
                                      "2. See Less Of My Feed\n"
                                      "3. Search For Users\n"
                                      "4. Search For Quacks\n"
                                      "5. Reply/Retweet From Feed\n"
                                      "6. List Followers\n"
                                      "7. CREATE NEW POST\n"
                                      "8. Log Out\n"
                                      "Selection: ";
    std::cin >> select;
    if (std::cin.peek() != '\n') select = '0';
    // Consume any trailing '\n' and discard it
    { std::string dummy; std::getline(std::cin, dummy); }
    switch (select) {
      case '1':
        FeedDisplayCount += 5;
        error = "";
        break;

      case '2':
        FeedDisplayCount -= 5;
        error = "";
        break;

      case '3':
        this->searchUsersPage();
        error = "";
        break;

      case '4':
        this->searchQuacksPage();
        error = "";
        break;

      case '5': {
          std::cout << "\nSelect a tweet (1,2,3,...) to reply/retweet or press Enter to return... ";
          std::string input;
          std::getline(std::cin, input);
          if (input.empty()) {
            break;
          }
          else {
            bool valid_input = false;
            while(!valid_input){
              std::regex positive_integer_regex("^[1-9]\\d*$");
              if (!std::regex_match(input, positive_integer_regex)) {
                  std::cout << "\033[A\033[2K" << std::flush;
                  std::cout << "Input Is Invalid: Select a tweet (1,2,3,...) to reply/retweet OR press Enter to return... ";
                  std::getline(std::cin, input);
                  if (input.empty()) {
                    break;
                  }
                  continue;
              }

              int32_t selection = std::stoi(input)-1;
              if (selection > static_cast<int32_t>(i-2) || selection < static_cast<int32_t>(i-6)) {
                  std::cout << "\033[A\033[2K" << std::flush;
                  std::cout << "Input Is Invalid: Select a tweet (1,2,3,...) to reply/retweet OR press Enter to return... ";
                  std::getline(std::cin, input);
                  if (input.empty()) {
                    break;
                  }
                  continue;
              }
              valid_input = true;

              if (valid_input) {
                this->quackPage(pond.getQuackFromID(this->feed_quack_ids[selection]));
              }
              break;
            }
            break;
          }
        }
      
      case '6':
        this->followersPage();
      break;

      case '7':
        postingPage();
        break;

      case '8':
        std::system("clear");
        FeedDisplayCount = 5;
        error = "";
        logged_in = false;
        delete this->_user_id;
        this->_user_id = nullptr;
        break;

      default:
        error = "\nInvalid Input Entered [use: 1, 2, 3, ..., 9].\n";
        break;
    }
  }
  
  startPage();
}

/**
 * @brief Allows the user to compose and post a new Quack.
 *
 * This method provides a user interface for creating and submitting a new Quack. 
 * Users can input their text or press Enter to exit without posting.
 *
 * @details
 * - Validates the input to ensure it is not empty before attempting to post.
 * - Attempts to post the Quack using the database. If successful, the user is notified.
 * - Handles errors during posting, such as issues with duplicate hashtags, and provides feedback.
 */
void Quacker::postingPage() {
  std::system("clear");
  std::string description = "Type your new Quack or press Enter to return.";
  std::string quack_text;
  while (true) {
    std::system("clear");
    std::cout << QUACKER_BANNER << "\n" << description << "\n\n--- New Quack ---\n";
    std::cout << "Enter your new quack: ";
    std::getline(std::cin, quack_text);
    quack_text = trim(quack_text);
    if (quack_text.empty()) {
      break;
    }
    if (pond.addQuack(*(this->_user_id), quack_text) != nullptr) {
      std::cout << "Quack posted successfully!\n";
      std::cout << "Press Enter to return... ";
      std::string input;
      std::getline(std::cin, input);
      while (!input.empty()) {
        std::cout << "\033[A\033[2K" << std::flush;
        std::cout << "Input Is Invalid: Press Enter to return... ";
        std::getline(std::cin, input);
      }
    } 
    else {
      description = "Error posting Quack, insure there are no duplicate hashtags and try again.";
    }
  }
}

/**
 * @brief Provides a user interface for searching and interacting with users.
 *
 * This method allows users to search for other users by name and displays the results.
 * Users can view more or fewer results, select a user to follow, or exit the search interface.
 *
 * @details
 * - Retrieves and displays search results based on the input query.
 * - Allows users to interact with search results by selecting a user to view or follow.
 * - Handles navigation through paginated search results.
 * - Ensures input validation for selection and provides appropriate feedback for invalid inputs.
 * - Allows users to exit the interface by pressing Enter without input.
 */
void Quacker::searchUsersPage() {
  std::string description = "Search for a user or press Enter to return.";
  while (true) {
    // show search interface
    std::system("clear");
    std::cout << QUACKER_BANNER << "\n" << description << "\n\n--- User Search ---\n";

    std::string search_term;
    std::cout << "Search for user name: ";
    std::getline(std::cin, search_term);
    search_term = trim(search_term);
    if (search_term.empty()) return;

    // query
    std::vector<Pond::User> results = pond.searchForUsers(search_term);

    // display results
    if (results.empty()) {
      std::cout << "No users found matching the search term.\n";
      std::cout << '\n' << '\n';
      std::cout << "Press Enter to return... ";
      std::string input;
      std::getline(std::cin, input);
      while (!input.empty()) {
        std::cout << "\033[A\033[2K" << std::flush;
        std::cout << "Input Is Invalid: Press Enter to return... ";
        std::getline(std::cin, input);
      }
    } else {
      int32_t i = 1;
      int32_t UserDisplayCount = 5;
      
      while(true){
        i = 1;
        std::cout << "Found " << results.size() << " users matching the search term.\n\n";

        for (const Pond::User& result : results) {
          ++i;
          if((UserDisplayCount < i-1 || i <= UserDisplayCount-4) && UserDisplayCount < static_cast<int32_t>(results.size())) continue;
          else if((i <= static_cast<int32_t>(results.size()-4)) && UserDisplayCount >= static_cast<int32_t>(results.size())) continue;

          std::ostringstream oss;
          oss << "----------------------------------------------------------------------------------------------------\n";
          oss << i-1 << ".\n";
          oss << "  User ID: " << std::setw(40) << std::left << result.usr
              << "Name: " << result.name << "\n\n";
          std::cout << oss.str();
        } std::cout << "----------------------------------------------------------------------------------------------------\n\n";
        if(5 > static_cast<int32_t>(results.size())){
          // Prompt the user to search again or return
          std::cout << "Select a user (1,2,3,...) to follow OR press Enter to return: ";
          std::string input;
          std::getline(std::cin, input);
          if (input.empty()) {
            break;
          }
          else {
            bool valid_input = false;
            while (!valid_input) {
              std::regex positive_integer_regex("^[1-9]\\d*$");
              if (!std::regex_match(input, positive_integer_regex)) {
                std::cout << "\033[A\033[2K" << std::flush;
                std::cout << "Input Is Invalid: Select a user (1,2,3,...) to follow OR press Enter to return: ";
                std::getline(std::cin, input);
                if (input.empty()) {
                  break;
                }
                continue;
              }

              int32_t selection = std::stoi(input) - 1;
              if (selection > i - 2) {
                std::cout << "\033[A\033[2K" << std::flush;
                std::cout << "Input Is Invalid: Select a user (1,2,3,...) to follow OR press Enter to return: ";
                std::getline(std::cin, input);
                if (input.empty()) {
                  break;
                }
                continue;
              }
              valid_input = true;
              
              if (selection <= static_cast<int32_t>(results.size())) {
                this->userPage(results[selection]);
              }
              break;
            }  
          }
          break;
        }
        else{

          // Prompt the user to search again or return
          std::cout << "Select a user (1,2,3,...) to follow, Enter M for more users, Enter L for less users OR press Enter to return: ";
          std::string input;
          std::getline(std::cin, input);
          if (input.empty()) {
            break;
          }
          else if (input == "M" || input == "m"){
            if (UserDisplayCount < static_cast<int32_t>(results.size())){
              UserDisplayCount +=5;
              if(UserDisplayCount !=5) std::cout << "\033[25A" << "\033[0J";
              else {
                std::cout << "\033[5A" << "\033[0J";
              }
            } 
            else{
              std::cout << "\033[A\033[2K" << std::flush;
              std::cout << "You Have No More Users To Display: Press Enter To Return: ";
              std::getline(std::cin, input);
              while (!input.empty()) {
                std::cout << "\033[A\033[2K" << std::flush;
                std::cout << "Invalid Input: Press Enter To Return: ";
                std::getline(std::cin, input);
              } std::cout << "\033[25A" << "\033[0J";
            }
            continue;
          }
          else if (input == "L" || input == "l"){
            if (UserDisplayCount > 0) {
              UserDisplayCount -=5;
              std::cout << "\033[25A" << "\033[0J";
            }
            else{
              std::cout << "\033[A\033[2K" << std::flush;
              std::cout << "You Are Already Showing No Users: Press Enter To Return: ";
              std::getline(std::cin, input);
              while (!input.empty()) {
                std::cout << "\033[A\033[2K" << std::flush;
                std::cout << "Invalid Input: Press Enter To Return: ";
                std::getline(std::cin, input);
              } std::cout << "\033[5A" << "\033[0J";
            }
            continue;
          }
          else {
            bool valid_input = false;
            while (!valid_input) {
              std::regex positive_integer_regex("^[1-9]\\d*$");
              if (!std::regex_match(input, positive_integer_regex)) {
                std::cout << "\033[A\033[2K" << std::flush;
                std::cout << "Input Is Invalid: Press Enter To Return: ";
                std::getline(std::cin, input);
                while (!input.empty()) {
                std::cout << "\033[A\033[2K" << std::flush;
                std::cout << "Invalid Input: Press Enter To Return: ";
                std::getline(std::cin, input);
                } input = "dont break pls";
                if (UserDisplayCount != 0) std::cout << "\033[25A" << "\033[0J";
                else std::cout << "\033[A\033[2K" << std::flush << "\033[4A" << "\033[0J";
                break;
              }

              int32_t selection = std::stoi(input) - 1;
              valid_input = true;
              
              if((selection+1 <= UserDisplayCount && selection+1 > UserDisplayCount-5) && UserDisplayCount < static_cast<int32_t>(results.size())){
                this->userPage(results[selection]);
                input = "";
                valid_input = true;
              }
              else if((selection+1 <= static_cast<int32_t>(results.size()) && (selection+1 > static_cast<int32_t>(results.size()-5)) && UserDisplayCount >= static_cast<int32_t>(results.size()))){
                this->userPage(results[selection]);
                input = "";
                valid_input = true;
              } else{
                std::cout << "\033[A\033[2K" << std::flush;
                std::cout << "Input Is Invalid: Press Enter To Return: ";
                std::getline(std::cin, input);
                while (!input.empty()) {
                std::cout << "\033[A\033[2K" << std::flush;
                std::cout << "Invalid Input: Press Enter To Return: ";
                std::getline(std::cin, input);
                } input = "dont break pls";
                if (UserDisplayCount != 0) std::cout << "\033[25A" << "\033[0J";
                else std::cout << "\033[A\033[2K" << std::flush << "\033[4A" << "\033[0J";
                break;
              }
              break;
            }
            if (input.empty()) break;
          }
        }
      }
    }
  }
}

/**
 * @brief Provides a user interface for searching and interacting with Quacks.
 *
 * This method allows users to search for Quacks using keywords or hashtags and 
 * displays the results. Users can navigate through results, reply to or requack 
 * posts, or return to the main menu.
 *
 * @details
 * - Retrieves and displays search results based on the entered search term.
 * - Supports pagination for viewing more or fewer Quacks in the results.
 * - Allows users to interact with Quacks by replying or requacking.
 * - Validates user input for result navigation and Quack interaction to ensure proper behavior.
 */
void Quacker::searchQuacksPage() {
  std::string description = "Search for a keyword or hashtag, or press Enter to return... ";
  while (true) {
    // show search interface
    std::system("clear");
    std::cout << QUACKER_BANNER << "\n" << description << "\n\n--- Quack Search ---\n";

    std::string search_term;
    std::cout << "Search for a Quack: ";
    std::getline(std::cin, search_term);
    search_term = trim(search_term);
    if (search_term.empty()) return;

    // query
    std::vector<Pond::Quack> results = pond.searchForQuacks(search_term);
   
    
    // display results
    if (results.empty()) {
      std::cout << "No Quacks found matching the search term.\n";
      std::cout << '\n' << '\n';
      std::cout << "Press Enter to return... ";
      std::string input;
      std::getline(std::cin, input);
      while (!input.empty()) {
        std::cout << "\033[A\033[2K" << std::flush;
        std::cout << "Input Is Invalid: Press Enter to return... ";
        std::getline(std::cin, input);
      }
    }
    else {
      int32_t QuackDisplayCount = 5;
      int32_t i = 1;

      std::cout << "Found " << results.size() << " Quacks matching the search term.\n";
      std::cout << '\n';
      for(int i = 0; i < 100; ++i) std::cout << '-';
      std::cout << '\n';
      while(true){
        i = 1; 

        for (const Pond::Quack& result : results) {
          ++i;

          if((QuackDisplayCount < i-1 || i <= QuackDisplayCount-4) && QuackDisplayCount < static_cast<int32_t>(results.size())) continue;
          else if((i <= static_cast<int32_t>(results.size()-4)) && QuackDisplayCount >= static_cast<int32_t>(results.size())) continue;

          std::ostringstream oss;
          oss << i-1 << ".\n";
          oss << "Quack ID: " << result.tid;
          oss << ", Author: " << ((pond.getUsername(result.writer_id) != "") ? pond.getUsername(result.writer_id) : "Unknown");
          oss << std::string(69 - oss.str().length(), ' ');
          oss << "Date and Time: " << (result.date.empty() ? "Unknown" : result.date);
          oss << " " << (result.time.empty() ? "Unknown" : result.time) << "\n\n";
          oss << "Text: " << formatTweetText(result.text, 94) << "\n";
          oss << "\n";
          for(int i = 0; i < 100; ++i) oss << '-'; 
          oss << '\n';
          std::cout << oss.str();
        }
        std::cout << '\n';

        if(5 > static_cast<int32_t>(results.size())){
          // Prompt the user to search again or return
          std::cout << "Select a quack (1,2,3,...) to reply/requack OR press Enter to return... ";
          std::string input;
          std::getline(std::cin, input);
          if (input.empty()) {
            break;
          }
          else {
            bool valid_input = false;
            while(!valid_input){
              std::regex positive_integer_regex("^[1-9]\\d*$");
              if (!std::regex_match(input, positive_integer_regex)) {
                  std::cout << "\033[A\033[2K" << std::flush;
                  std::cout << "Input Is Invalid: Select a quack (1,2,3,...) to reply/requack OR press Enter to return... ";
                  std::getline(std::cin, input);
                  if (input.empty()) {
                    break;
                  }
                  continue;
              }

              int32_t selection = std::stoi(input)-1;
              if (selection > i-2) {
                  std::cout << "\033[A\033[2K" << std::flush;
                  std::cout << "Input Is Invalid: Select a quack (1,2,3,...) to reply/requack OR press Enter to return... ";
                  std::getline(std::cin, input);
                  if (input.empty()) {
                    break;
                  }
                  continue;
              }
              valid_input = true;
            
              if (selection <= static_cast<int32_t>(results.size())) {
                this->quackPage(results[selection]);
              }
              break;
            }
          }
          break;
        }
        else{
          // Prompt the user to search again or return
          std::cout << "Select a quack (1,2,3,...) to reply/requack, Enter M for more quacks, Enter L for less quacks OR press Enter to return... ";
          std::string input;
          std::getline(std::cin, input);
          if (input.empty()) {
            break;
          }
          else if (5 > static_cast<int32_t>(results.size()));
          else if (input == "M" || input == "m"){
            if (QuackDisplayCount < static_cast<int32_t>(results.size())){
              QuackDisplayCount +=5;
              if(QuackDisplayCount !=5) std::cout << "\033[32A" << "\033[0J";
              else {
                std::cout << "\033[2A" << "\033[0J";
              }
            } 
            else{
              std::cout << "\033[A\033[2K" << std::flush;
              std::cout << "You Have No More Quacks To Display: Press Enter To Return: ";
              std::getline(std::cin, input);
              while (!input.empty()) {
                std::cout << "\033[A\033[2K" << std::flush;
                std::cout << "Invalid Input: Press Enter To Return: ";
                std::getline(std::cin, input);
              } std::cout << "\033[32A" << "\033[0J";
            }
            continue;
          }
          else if (input == "L" || input == "l"){
            if (QuackDisplayCount > 0) {
              QuackDisplayCount -=5;
              std::cout << "\033[32A" << "\033[0J";
            }
            else{
              std::cout << "\033[A\033[2K" << std::flush;
              std::cout << "You Are Already Showing No Quacks: Press Enter To Return: ";
              std::getline(std::cin, input);
              while (!input.empty()) {
                std::cout << "\033[A\033[2K" << std::flush;
                std::cout << "Invalid Input: Press Enter To Return: ";
                std::getline(std::cin, input);
              } std::cout << "\033[2A" << "\033[0J";
            }
            continue;
          }
          else {
            bool valid_input = false;
            while (!valid_input) {
              std::regex positive_integer_regex("^[1-9]\\d*$");
              if (!std::regex_match(input, positive_integer_regex)) {
                std::cout << "\033[A\033[2K" << std::flush;
                std::cout << "Input Is Invalid: Press Enter To Return: ";
                std::getline(std::cin, input);
                while (!input.empty()) {
                std::cout << "\033[A\033[2K" << std::flush;
                std::cout << "Invalid Input: Press Enter To Return: ";
                std::getline(std::cin, input);
                } input = "dont break pls";
                if (QuackDisplayCount != 0) std::cout << "\033[32A" << "\033[0J";
                else std::cout << "\033[A\033[2K" << std::flush << "\033[1A" << "\033[0J";
                break;
              }

              int32_t selection = std::stoi(input) - 1;
              valid_input = true;
              
              if((selection+1 <= QuackDisplayCount && selection+1 > QuackDisplayCount-5) && QuackDisplayCount < static_cast<int32_t>(results.size())){
                this->quackPage(results[selection]);
                input = "";
                valid_input = true;
              }
              else if((selection+1 <= static_cast<int32_t>(results.size()) && (selection+1 > static_cast<int32_t>(results.size()-5)) && QuackDisplayCount >= static_cast<int32_t>(results.size()))){
                this->quackPage(results[selection]);
                input = "";
                valid_input = true;
              } else{
                std::cout << "\033[A\033[2K" << std::flush;
                std::cout << "Input Is Invalid: Press Enter To Return: ";
                std::getline(std::cin, input);
                while (!input.empty()) {
                std::cout << "\033[A\033[2K" << std::flush;
                std::cout << "Invalid Input: Press Enter To Return: ";
                std::getline(std::cin, input);
                } input = "dont break pls";
                if (QuackDisplayCount != 0) std::cout << "\033[32A" << "\033[0J";
                else std::cout << "\033[A\033[2K" << std::flush << "\033[1A" << "\033[0J";
                break;
              }
              break;
            }
            if (input.empty()) break;
          }
        }
      }
    }
  }
}

/**
 * @brief Displays a detailed user profile and allows interactions with the user's content.
 *
 * This method provides an interface to view a user's profile, including their information, 
 * quacks, and follower statistics. Users can navigate through the profile, view more or fewer 
 * quacks, follow the user, or return to the previous menu.
 *
 * @details
 * - Displays the selected user's profile details, including name, follower count, and quack count.
 * - Fetches and displays the user's quacks, with pagination to show more or fewer quacks.
 * - Provides an option to follow the user, with validation to prevent self-following or duplicate follows.
 * - Handles user input to navigate or interact with the profile and validates it for accuracy.
 */
 void Quacker::userPage(const Pond::User& user) {
  int32_t user_id = *(this->_user_id);
  std::string error = "";
  int32_t hardstop = 3;
  while (true) {
    int32_t i = 1;
    std::system("clear");
    char select;
    std::cout << QUACKER_BANNER;
    std::cout << "\nActions For User:\n\n";
    std::ostringstream oss;
    oss << "----------------------------------------------------------------------------------------------------\n";
    oss << "  User ID: " << std::setw(40) << std::left << user.usr
        << "Name: " << user.name << "\n";
    oss << "  Followers: " << std::setw(38) << std::left << pond.getFollowers(user.usr).size()
        << "Follows: " << pond.getFollows(user.usr).size() << "\n  Quack Count: " << pond.getQuacks(user.usr).size() << "\n\n";
    std::cout << oss.str();
    std::cout << "------------------------------------------- User's Quacks ------------------------------------------\n\n";
    
    std::vector<Pond::Quack> users_quacks = pond.getQuacks(user.usr);

    for (const Pond::Quack& result : users_quacks) {
        ++i;
        if(i-1 > hardstop) break;
        if(hardstop >= static_cast<int32_t>(pond.getQuacks(user.usr).size())) {
          if((i-1 <= (static_cast<int32_t>(pond.getQuacks(user.usr).size()-3)))) continue;
        } else if((i-1 <= (hardstop-3))) continue;
        std::ostringstream oss;
        
        oss << i-1 << ".\n";
        oss << "Quack ID: " << result.tid;
        oss << ", Author: " << ((pond.getUsername(result.writer_id) != "") ? pond.getUsername(result.writer_id) : "Unknown");
        oss << std::string(69 - oss.str().length(), ' ');
        oss << "Date and Time: " << (result.date.empty() ? "Unknown" : result.date);
        oss << " " << (result.time.empty() ? "Unknown" : result.time) << "\n\n";
        oss << "Text: " << formatTweetText(result.text, 94) << "\n";

        oss << "\n";
        for(int i = 0; i < 100; ++i) oss << '-'; 
        oss << '\n';
        std::cout << oss.str();
      }

    std::cout << error <<
      "\n\n1. See More Of The Users Quacks"
      "\n2. See Less Of The Users Quacks"
      "\n3. Follow The User"
      "\n4. Reply/Requack To a Quack" 
      "\n5. Return"
      "\n\nSelection: ";
    std::cin >> select;
    if (std::cin.peek() != '\n') select = '0';
    // Consume any trailing '\n' and discard it
    { std::string dummy; std::getline(std::cin, dummy); }
    switch (select) {
      case '1':
        error = "";
        hardstop += 3;
        if (static_cast<long unsigned int>(hardstop) >= users_quacks.size() + 3){
          error = "\nThis User Has No More Quacks To Diplay!";
          hardstop -= 3;
          break;
        }
        break;
      case '2':
        error = "";
        if (hardstop == 0){
          error = "You Are Already Not Seeing Any Quacks!";
          break;
        }
        hardstop -= 3;
        break;
      case '3': 
        {
          error = "";
          bool already_follows = false;
          for (int32_t flws : pond.getFollows(user_id)) {
            if (flws == user.usr || user_id == user.usr) { 
              if (flws == user.usr) std::cout << "You already follow " << user.name << "\n";
              if (user_id == user.usr) std::cout << "You can't follow yourself " << user.name << "\n";
              std::cout << "Press Enter to return... ";
              std::string input;
              std::getline(std::cin, input);
              while (!input.empty()) {
                std::cout << "\033[A\033[2K" << std::flush;
                std::cout << "Input Is Invalid: Press Enter to return... ";
                std::getline(std::cin, input);
              }
              already_follows = true;
              break;
            }
          }
          if (!already_follows) {
            pond.follow(user_id, user.usr);
            std::cout << "You are now following " << user.name << "\n";
            std::cout << "Press Enter to return... ";
            std::string input;
            std::getline(std::cin, input);
            while (!input.empty()) {
              std::cout << "\033[A\033[2K" << std::flush;
              std::cout << "Input Is Invalid: Press Enter to return... ";
              std::getline(std::cin, input);
            }
          }
        }
        break;
      case '4':{
          std::cout << "\nSelect a tweet (1,2,3,...) to reply/retweet or press Enter to return... ";
          std::string input;
          std::getline(std::cin, input);
          if (input.empty()) {
            break;
          }
          else {
            bool valid_input = false;
            while(!valid_input){
              std::regex positive_integer_regex("^[1-9]\\d*$");
              if (!std::regex_match(input, positive_integer_regex)) {
                  std::cout << "\033[A\033[2K" << std::flush;
                  std::cout << "Input Is Invalid: Select a tweet (1,2,3,...) to reply/retweet OR press Enter to return... ";
                  std::getline(std::cin, input);
                  if (input.empty()) {
                    break;
                  }
                  continue;
              }

              int32_t selection = std::stoi(input)-1;
              if (selection > static_cast<int32_t>(i-2) || selection < static_cast<int32_t>(i-6)) {
                  std::cout << "\033[A\033[2K" << std::flush;
                  std::cout << "Input Is Invalid: Select a tweet (1,2,3,...) to reply/retweet OR press Enter to return... ";
                  std::getline(std::cin, input);
                  if (input.empty()) {
                    break;
                  }
                  continue;
              }
              valid_input = true;

              if (valid_input) {
                this->quackPage(users_quacks[selection]);
              }
              break;
            }
            break;
          }
        }
        return;
      case '5':
        error = "";
        return;
      default:
        error = "\nInvalid Input Entered [use: 1, 2, 3].\n";
        break;
    }
  }
}

/**
 * @brief Provides an interface for replying to a specific Quack.
 *
 * This method allows a user to write and post a reply to a given Quack. The 
 * details of the Quack being replied to are displayed, and the user can submit 
 * their reply or cancel the action.
 *
 * @details
 * - Displays the original Quack details, including text, author, and timestamps.
 * - Allows users to compose a reply, which is then added to the database.
 * - Handles errors during reply submission and provides appropriate feedback.
 * - Users can exit the reply interface by pressing Enter without entering text.
 */
void Quacker::replyPage(const Pond::Quack& reply) {
  const int32_t user_id = *(this->_user_id);
  std::string error = "";
  while (true) {
    std::system("clear");
    std::cout << QUACKER_BANNER;
    std::cout << "\nReply For Quack:\n\n";
    
    for(int i = 0; i < 100; ++i) std::cout << '-';
    std::ostringstream oss;
    
    oss << "\nQuack ID: " << reply.tid;
    oss << ", Author: " << ((pond.getUsername(reply.writer_id) != "") ? pond.getUsername(reply.writer_id) : "Unknown");
    oss << std::string(67 - oss.str().length(), ' ');
    oss << "Date and Time: " << (reply.date.empty() ? "Unknown" : reply.date);
    oss << " " << (reply.time.empty() ? "Unknown" : reply.time) << "\n\n";
    oss << "Text: " << formatTweetText(reply.text, 94) << "\n\n";
    oss << "Requack Count: " << pond.getRequackCount(reply.tid) << "     Reply Count: " << pond.getReplies(reply.tid).size() << "\n\n";

    std::cout << oss.str();
    
    for(int i = 0; i < 100; ++i) std::cout << '-';

    std::string reply_text;
    std::cout << "\n\nEnter your reply or press Enter to cancel: ";
    std::getline(std::cin, reply_text);
    if (reply_text.empty()) return;
    if (pond.addReply(user_id, reply.tid, reply_text)) {
      std::cout << "\nReply posted successfully!\n";
      std::cout << "Press Enter to return... ";
      std::string input;
      std::getline(std::cin, input);
      while (!input.empty()) {
        std::cout << "\033[A\033[2K" << std::flush;
        std::cout << "Input Is Invalid: Press Enter to return... ";
        std::getline(std::cin, input);
      } if(input.empty()) break;
    }
    else {
      error = "Error posting reply, please try again.";
    }
  }
}

/**
 * @brief Provides an interface for interacting with a specific Quack.
 *
 * This method allows a user to view details about a Quack and take actions such as 
 * replying, requacking, or returning to the previous menu.
 *
 * @details
 * - Displays detailed information about the Quack, including its author, content, and metadata.
 * - Users can reply to the Quack, which redirects to the reply interface.
 * - Users can requack the post, with validation to prevent duplicate requacks.
 * - Handles errors during requacking and provides feedback.
 * - Allows users to exit the interface by selecting the return option.
 */
void Quacker::quackPage(const Pond::Quack& reply) {
  const int32_t user_id = *(this->_user_id);
  std::string error = "";
  while (true) {
    std::system("clear");
    char select;
    std::cout << QUACKER_BANNER;
    std::cout << "\nActions For Quack:\n\n";
    
    for(int i = 0; i < 100; ++i) std::cout << '-';
    std::ostringstream oss;
    
    oss << "\nQuack ID: " << reply.tid;
    oss << ", Author: " << ((pond.getUsername(reply.writer_id) != "") ? pond.getUsername(reply.writer_id) : "Unknown");
    oss << std::string(67 - oss.str().length(), ' ');
    oss << "Date and Time: " << (reply.date.empty() ? "Unknown" : reply.date);
    oss << " " << (reply.time.empty() ? "Unknown" : reply.time) << "\n\n";
    oss << "Text: " << formatTweetText(reply.text, 94) << "\n\n";
    oss << "Requack Count: " << pond.getRequackCount(reply.tid) << "     Reply Count: " << pond.getReplies(reply.tid).size() << "\n\n";

    std::cout << oss.str();
    
    for(int i = 0; i < 100; ++i) std::cout << '-';

    std::cout << error <<
      "\n\n1. Reply"
      "\n2. Requack"
      "\n3. Return"
      "\n\nSelection: ";
    std::cin >> select;
    if (std::cin.peek() != '\n') select = '0';
    // Consume any trailing '\n' and discard it
    { std::string dummy; std::getline(std::cin, dummy); }
    switch (select) {
      case '1':
        error = "";
        this->replyPage(reply);
        break;
      case '2': {
        error = "";
        int32_t joebiden = pond.addRequack(user_id, reply.tid);
        if (joebiden == 0) {
          std::cout << "Requack successful!\n";
          std::cout << "Press Enter to return... ";
          std::string input;
          std::getline(std::cin, input);
          while (!input.empty()) {
            std::cout << "\033[A\033[2K" << std::flush;
            std::cout << "Input Is Invalid: Press Enter to return... ";
            std::getline(std::cin, input);
          }
        }
        else if (joebiden == 1) {
          error = "\n\nYou've already requacked this, marked as spam...\n";
        }
        else{
          error = "\n\nError requacking, please try again.\n";
        }
        break;
    }
      case '3':
        error = "";
        return;
      default:
        error = "\n\nInvalid Input Entered [use: 1, 2, 3].\n";
        break;
    }
  }
}

/**
 * @brief Displays the list of followers and allows interaction with the follower profiles.
 *
 * This method shows the user's followers and provides options to navigate through the list
 * or view detailed profiles of individual followers.
 *
 * @details
 * - Retrieves and displays a list of followers associated with the logged-in user.
 * - Supports pagination for viewing more or fewer followers at a time.
 * - Allows users to select a follower from the list to view their profile.
 * - Validates user input for navigation and profile selection.
 * - Handles cases where there are no followers gracefully by displaying an appropriate message.
 */
void Quacker::followersPage() {
  std::string description = "View your followers or press Enter to return.";
  
  // show search interface
  std::system("clear");
  std::cout << QUACKER_BANNER << "\n" << description << "\n\n--- Your Followers ---\n";

  // query
  std::vector<Pond::User> results = pond.getFollowers(*(this->_user_id));

  // display results
  if (results.empty()) {
    std::cout << "You Do Not Follow Anyone :(\n";
    std::cout << '\n' << '\n';
    std::cout << "Press Enter to return... ";
    std::string input;
    std::getline(std::cin, input);
    while (!input.empty()) {
      std::cout << "\033[A\033[2K" << std::flush;
      std::cout << "Input Is Invalid: Press Enter to return... ";
      std::getline(std::cin, input);
    }
  } else {
    int32_t i = 1;
    int32_t UserDisplayCount = 5;
    
    while(true){
      i = 1;
      std::cout << "Found " << results.size() << " Users You Follow :)\n\n";

      for (const Pond::User& result : results) {
        ++i;
        if((UserDisplayCount < i-1 || i <= UserDisplayCount-4) && UserDisplayCount < static_cast<int32_t>(results.size())) continue;
        else if((i <= static_cast<int32_t>(results.size()-4)) && UserDisplayCount >= static_cast<int32_t>(results.size())) continue;

        std::ostringstream oss;
        oss << "----------------------------------------------------------------------------------------------------\n";
        oss << i-1 << ".\n";
        oss << "  User ID: " << std::setw(40) << std::left << result.usr
            << "Name: " << result.name << "\n\n";
        std::cout << oss.str();
      } std::cout << "----------------------------------------------------------------------------------------------------\n\n";
      if(5 > static_cast<int32_t>(results.size())){
        // Prompt the user to search again or return
        std::cout << "Select a user (1,2,3,...) to follow OR press Enter to return: ";
        std::string input;
        std::getline(std::cin, input);
        if (input.empty()) {
          break;
        }
        else {
          bool valid_input = false;
          while (!valid_input) {
            std::regex positive_integer_regex("^[1-9]\\d*$");
            if (!std::regex_match(input, positive_integer_regex)) {
              std::cout << "\033[A\033[2K" << std::flush;
              std::cout << "Input Is Invalid: Select a user (1,2,3,...) to follow OR press Enter to return: ";
              std::getline(std::cin, input);
              if (input.empty()) {
                break;
              }
              continue;
            }

            int32_t selection = std::stoi(input) - 1;
            if (selection > i - 2) {
              std::cout << "\033[A\033[2K" << std::flush;
              std::cout << "Input Is Invalid: Select a user (1,2,3,...) to follow OR press Enter to return: ";
              std::getline(std::cin, input);
              if (input.empty()) {
                break;
              }
              continue;
            }
            valid_input = true;
            
            if (selection <= static_cast<int32_t>(results.size())) {
              this->userPage(results[selection]);
            }
            break;
          }  
        }
        break;
      }
      else{

        // Prompt the user to search again or return
        std::cout << "Select a user (1,2,3,...) to follow, Enter M for more users, Enter L for less users OR press Enter to return: ";
        std::string input;
        std::getline(std::cin, input);
        if (input.empty()) {
          break;
        }
        else if (input == "M" || input == "m"){
          if (UserDisplayCount < static_cast<int32_t>(results.size())){
            UserDisplayCount +=5;
            if(UserDisplayCount !=5) std::cout << "\033[25A" << "\033[0J";
            else {
              std::cout << "\033[5A" << "\033[0J";
            }
          } 
          else{
            std::cout << "\033[A\033[2K" << std::flush;
            std::cout << "You Have No More Users To Display: Press Enter To Return: ";
            std::getline(std::cin, input);
            while (!input.empty()) {
              std::cout << "\033[A\033[2K" << std::flush;
              std::cout << "Invalid Input: Press Enter To Return: ";
              std::getline(std::cin, input);
            } std::cout << "\033[25A" << "\033[0J";
          }
          continue;
        }
        else if (input == "L" || input == "l"){
          if (UserDisplayCount > 0) {
            UserDisplayCount -=5;
            std::cout << "\033[25A" << "\033[0J";
          }
          else{
            std::cout << "\033[A\033[2K" << std::flush;
            std::cout << "You Are Already Showing No Users: Press Enter To Return: ";
            std::getline(std::cin, input);
            while (!input.empty()) {
              std::cout << "\033[A\033[2K" << std::flush;
              std::cout << "Invalid Input: Press Enter To Return: ";
              std::getline(std::cin, input);
            } std::cout << "\033[5A" << "\033[0J";
          }
          continue;
        }
        else {
          bool valid_input = false;
          while (!valid_input) {
            std::regex positive_integer_regex("^[1-9]\\d*$");
            if (!std::regex_match(input, positive_integer_regex)) {
              std::cout << "\033[A\033[2K" << std::flush;
              std::cout << "Input Is Invalid: Press Enter To Return: ";
              std::getline(std::cin, input);
              while (!input.empty()) {
              std::cout << "\033[A\033[2K" << std::flush;
              std::cout << "Invalid Input: Press Enter To Return: ";
              std::getline(std::cin, input);
              } input = "dont break pls";
              if (UserDisplayCount != 0) std::cout << "\033[25A" << "\033[0J";
              else std::cout << "\033[A\033[2K" << std::flush << "\033[4A" << "\033[0J";
              break;
            }

            int32_t selection = std::stoi(input) - 1;
            valid_input = true;
            
            if((selection+1 <= UserDisplayCount && selection+1 > UserDisplayCount-5) && UserDisplayCount < static_cast<int32_t>(results.size())){
              this->userPage(results[selection]);
              input = "";
              valid_input = true;
            }
            else if((selection+1 <= static_cast<int32_t>(results.size()) && (selection+1 > static_cast<int32_t>(results.size()-5)) && UserDisplayCount >= static_cast<int32_t>(results.size()))){
              this->userPage(results[selection]);
              input = "";
              valid_input = true;
            } else{
              std::cout << "\033[A\033[2K" << std::flush;
              std::cout << "Input Is Invalid: Press Enter To Return: ";
              std::getline(std::cin, input);
              while (!input.empty()) {
              std::cout << "\033[A\033[2K" << std::flush;
              std::cout << "Invalid Input: Press Enter To Return: ";
              std::getline(std::cin, input);
              } input = "dont break pls";
              if (UserDisplayCount != 0) std::cout << "\033[25A" << "\033[0J";
              else std::cout << "\033[A\033[2K" << std::flush << "\033[4A" << "\033[0J";
              break;
            }
            break;
          }
          if (input.empty()) break;
        }
      }
    }
  }
}

/**
 * @brief Processes and formats the current user's feed for display.
 *
 * This method fetches the feed for the logged-in user, formats it for output,
 * and handles pagination and display limits based on the `FeedDisplayCount`.
 *
 * @details
 * - Retrieves the feed content for the logged-in user.
 * - Formats the feed into a string for display, with appropriate indexing and delimiters.
 * - Handles pagination:
 *   - If `FeedDisplayCount` exceeds the available Quacks, adjusts it and sets an error message.
 *   - Ensures `FeedDisplayCount` does not go below zero.
 *   - Limits displayed Quacks to the requested count or the maximum available.
 * - Populates a list of visible Quack IDs for interaction with displayed items.
 *
 * @param FeedDisplayCount The number of Quacks to display, adjusted as needed.
 * @param error A reference to an error message string, set if display limits are exceeded.
 * @param i A reference to a counter for Quack indexing, updated during processing.
 * @return A formatted string representing the visible portion of the feed.
 */
std::string Quacker::processFeed(int32_t& FeedDisplayCount, std::string& error, int32_t& i) {
    const std::int32_t user_id = *(this->_user_id);
    std::vector<std::string> feed = pond.getFeed(user_id);

    int32_t maxQuacks = feed.size();
    i = 1;
    this->feed_quack_ids.clear();
    if (FeedDisplayCount >= maxQuacks + 5) {
        // Case 1: FeedDisplayCount is 5 or more beyond the available quacks
        error = "\nYou Have No More Quacks Left To Display.\n";
        FeedDisplayCount = std::max(0, static_cast<int>(FeedDisplayCount) - 5);
        std::ostringstream oss;
        while(i-1 < maxQuacks) {
            this->feed_quack_ids.push_back(extractQuackID(feed[i-1]));
            ++i;
            if((maxQuacks-5) >= i-1) continue;;
            oss << i-1 << ".\n";
            oss << feed[i-2] << "\n";
            for(int i = 0; i < 100; ++i) oss << '-'; 
            oss << '\n';
        }
        return oss.str();
    } else if (FeedDisplayCount >= maxQuacks && FeedDisplayCount <= maxQuacks + 4) {
        // Case 2: FeedDisplayCount is between maxQuacks and maxQuacks + 4
        // Do not modify FeedDisplayCount, but limit the display to maxQuacks
    } else if (FeedDisplayCount < maxQuacks && FeedDisplayCount > 0) {
        // Case 3: FeedDisplayCount is between 0 and maxQuacks
        // FeedDisplayCount remains as is
    } else if (FeedDisplayCount <= 0) {
        // Case 4: FeedDisplayCount is less than zero
        
        if(FeedDisplayCount != 0) error = "\nYou Are Already Not Displaying Any Quacks.\n";
        FeedDisplayCount = 0;
        return "";
    }

    int32_t displayCount = std::min(FeedDisplayCount, maxQuacks);

    std::ostringstream oss;
    while(i-1 < displayCount) {
        this->feed_quack_ids.push_back(extractQuackID(feed[i-1]));
        ++i;
        if((displayCount-5) >= i-1) continue;;
        oss << i-1 << ".\n";
        oss << feed[i-2] << "\n";
        for(int i = 0; i < 100; ++i) oss << '-'; 
        oss << '\n';
    }

    return oss.str();
}


/**
 * @brief Captures a password input without displaying it on the screen.
 *
 * This method allows secure input of a password by disabling terminal echo 
 * and handling backspace functionality for editing the input.
 *
 * @details
 * - Temporarily modifies terminal settings to disable echo and canonical mode.
 * - Captures each character typed by the user and replaces it with an asterisk (*) on the screen.
 * - Supports backspace functionality to delete characters from the input.
 * - Restores the original terminal settings after the input is complete.
 *
 * @return The entered password as a string.
 */
std::string Quacker::getHiddenPassword() {
  struct termios oldt, newt;
  std::string password;
  char ch;

  // save old terminal settings and disable echo
  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ECHO); // disable echo
  newt.c_lflag &= ~(ICANON); // disable canonical mode 
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);

  while (true) {
    ch = getchar();
    if (ch == '\n') { 
      std::cout << std::endl;
      break;
    }
    else if (ch == 127 || ch == '\b') { // backspace
      if (!password.empty()) {
        password.pop_back();
        std::cout << "\b \b"; 
      }
    }
    else {
      password.push_back(ch);
      std::cout << '*'; 
    }
  }

  // restore old terminal settings
  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  return password;
}

/**
 * @brief Validates a phone number string and returns its numeric value.
 *
 * Extracts digits from the input string and checks if it contains 10 or 11 digits,
 * excluding any newline or carriage return characters. Returns the numeric value
 * if valid, otherwise returns -1.
 *
 * @param input The phone number string to validate.
 * @return The numeric value of the phone number if valid, -1 if invalid.
 */
int64_t Quacker::isValidPhoneNumber(const std::string& input) {
  if (input.find('\n') != std::string::npos || input.find('\r') != std::string::npos) {
    return -1;
  }

  std::string digits;

  for (char c : input) {
    if (std::isdigit(c)) {
      digits += c;
    }
  }

  if (digits.size() < 10 || digits.size() > 11) {
    return -1;
  }

  try {
    return std::stoll(digits);
  } catch (const std::out_of_range&) {
    return -1;
  }
}

/**
 * @brief Validates an email address format.
 *
 * This function checks if the given email string adheres to a standard email
 * format and ensures it does not contain any newline or carriage return
 * characters, which are considered invalid.
 *
 * @param email The email address to validate.
 * @return true if the email address is in a valid format, false otherwise.
 *
 * @note The regex used is sourced from https://emailregex.com/
 */
bool Quacker::isValidEmail(const std::string& email) {
  if (email.find('\n') != std::string::npos || email.find('\r') != std::string::npos) {
    return false;
  }

  const std::regex email_pattern(R"((?:[a-z0-9!#$%&'*+/=?^_`{|}~-]+(?:\.[a-z0-9!#$%&'*+/=?^_`{|}~-]+)*|"(?:[\x01-\x08\x0b\x0c\x0e-\x1f\x21\x23-\x5b\x5d-\x7f]|\\[\x01-\x09\x0b\x0c\x0e-\x7f])*")@(?:(?:[a-z0-9](?:[a-z0-9-]*[a-z0-9])?\.)+[a-z0-9](?:[a-z0-9-]*[a-z0-9])?|\[(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?|[a-z0-9-]*[a-z0-9]:(?:[\x01-\x08\x0b\x0c\x0e-\x1f\x21-\x5a\x53-\x7f]|\\[\x01-\x09\x0b\x0c\x0e-\x7f])+)\]))");
  return std::regex_match(email, email_pattern);
}

/**
 * @brief Determines if a string is a valid integer ID.
 *
 * If the entire
 * string is successfully parsed as an integer (i.e., no non-numeric
 * characters remain), it is considered a valid ID.
 *
 * @param str The string to check.
 * @return true if the string is a valid integer, false otherwise.
 */
bool Quacker::isID(std::string str) {
  char* p;
  strtol(str.c_str(), &p, 10);
  return *p == 0;
}

/**
 * @brief Trims leading and trailing whitespace from a string.
 * 
 * @param str The string to be trimmed.
 * @return A new string with leading and trailing whitespace removed.
 * 
 * @note This function does not modify the original string but instead returns a new string with the whitespace removed.
 */
std::string Quacker::trim(const std::string& str) {
  // Find the first non-whitespace character from the start
  auto start = std::find_if_not(str.begin(), str.end(), [](unsigned char ch) {
      return std::isspace(ch);
  });

  // Find the last non-whitespace character from the end
  auto end = std::find_if_not(str.rbegin(), str.rend(), [](unsigned char ch) {
      return std::isspace(ch);
  }).base();

  // Return the trimmed string
  return (start < end) ? std::string(start, end) : std::string();
}

/**
 * @brief Formats a given text to wrap lines at a specified width.
 *
 * This method formats a string by inserting line breaks to ensure that no line
 * exceeds the specified `line_width`. Words are kept intact and wrapped cleanly 
 * to the next line if necessary.
 *
 * @param text The input string to be formatted.
 * @param line_width The maximum width of each line.
 * @return A formatted string with line breaks added as necessary.
 */
std::string Quacker::formatTweetText(const std::string& text, int line_width) {
    std::istringstream words(text);  // Stream to split text into words
    std::string word;
    std::ostringstream formattedText;
    int currentLineLength = 0;

    while (words >> word) {
        if (currentLineLength + word.length() + 1 > static_cast<std::string::size_type>(line_width)) {
            formattedText << "\n";
            currentLineLength = 0;
        }

        if (currentLineLength > 0) {
            formattedText << " ";
            currentLineLength++;
        }

        formattedText << word;
        currentLineLength += word.length();
    }

    return formattedText.str();
}

/**
 * @brief Extracts the Quack ID from a given string.
 *
 * This method parses a string to extract an integer ID prefixed by "Quack Id: ".
 * If the string does not start with the required prefix or the ID is invalid, 
 * an error message is displayed, and a default value is returned.
 *
 * @details
 * - Checks if the input string starts with the prefix "Quack Id: ".
 * - Uses a regular expression to extract the integer ID that follows the prefix.
 * - Handles errors where the prefix is missing or the ID is not a valid integer.
 *
 * @param quackString The input string containing the Quack ID.
 * @return The extracted Quack ID as an `int32_t`, or -1 if extraction fails.
 */
int32_t Quacker::extractQuackID(const std::string& quackString) {
    const std::string prefix = "Quack Id: ";
    
    if (quackString.find(prefix) == 0) {
        std::regex pattern(R"(^Quack Id:\s+(\d+))");
        std::smatch match;

        if (std::regex_search(quackString, match, pattern)) {
            return static_cast<int32_t>(std::stoi(match[1].str()));
        } else {
            std::cerr << "Error: No valid integer found after 'Quack Id: '" << std::endl;
            return -1;
        }
    } else {
        std::cerr << "Error: String does not start with 'Quack Id: '" << std::endl;
        return -1;
    }
}