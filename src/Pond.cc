#include "Pond.hh"

#include <sstream>


Pond::Pond()
  : _db(nullptr) {
}

Pond::~Pond() {
  sqlite3_close(this->_db);
}

/**
 * @brief Opens a connection to the SQLite database specified by the filename.
 *
 * @param db_filename The name of the database file to open.
 * @return int Returns SQLITE_OK (0) if the database was successfully opened,
 *         or a non-zero SQLite error code if it failed.
 */
int Pond::loadDatabase(const std::string& db_filename) {
  int exit_code = sqlite3_open(db_filename.c_str(), &this->_db);
  if (exit_code) {
    std::cerr << "Can't open database: " << sqlite3_errmsg(this->_db) << std::endl;
    return exit_code;
  }
  return 0;
}

/**
 * @brief Adds a new user to the users table in the database.
 *
 * @param user_id The unique ID of the user.
 * @param name The name of the user.
 * @param email The email of the user.
 * @param phone The phone number of the user.
 * @param password The password for the user's account.
 * @return true if the user was successfully added; false otherwise.
 */
int32_t* Pond::addUser(const std::string& name, const std::string& email, const int64_t& phone, const std::string& password) {
  int32_t user_id;
  
  // Get a unique user ID
  if (!get_unique_user_id(user_id)) {
    return nullptr;  // Return nullptr if we couldn't get a unique ID
  }
  
  const char* query = 
    "INSERT INTO users (usr, name, email, phone, pwd) "
    "VALUES (?, ?, ?, ?, ?)";
  
  sqlite3_stmt* stmt;
  if (sqlite3_prepare_v2(this->_db, query, -1, &stmt, nullptr) != SQLITE_OK) {
    sqlite3_finalize(stmt);
    return nullptr;
  }

  // Bind parameters to prevent SQL injection.
  sqlite3_bind_int(stmt,  1, user_id);                              // usr
  sqlite3_bind_text(stmt, 2, name.c_str(), -1, SQLITE_STATIC);      // name
  sqlite3_bind_text(stmt, 3, email.c_str(), -1, SQLITE_STATIC);     // email
  sqlite3_bind_int(stmt,  4, phone);                                // phone
  sqlite3_bind_text(stmt, 5, password.c_str(), -1, SQLITE_STATIC);  // pwd

  // Execute the query.
  int32_t* result = nullptr;
  if (sqlite3_step(stmt) == SQLITE_DONE) {
    result = new int32_t(user_id);  // Allocate a new int32_t if user was added successfully
  }
  
  sqlite3_finalize(stmt);
  return result;  // Return either the pointer to user_id or nullptr
}

/**
 * @brief Adds a new post to the posts table in the database.
 *
 * @param tweet_id The unique ID of the tweet.
 * @param user_id The ID of the user who created the post.
 * @param text The text content of the post.
 * @return true if the post was successfully added; false otherwise.
 */
bool Pond::addPost(const int32_t& tweet_id, const int32_t& user_id, const std::string& text) {
  bool post_added = false;

  const char* query = 
    "INSERT INTO posts (tid, writer_id, text, tdate, ttime) "
    "VALUES (?, ?, ?, ?, ?)";

  // Prepare the SQL statement.
  sqlite3_stmt* stmt;
  if (sqlite3_prepare_v2(this->_db, query, -1, &stmt, nullptr) != SQLITE_OK) {
    sqlite3_finalize(stmt);
    return false;
  }

  // Bind parameters to prevent SQL injection.
  sqlite3_bind_int(stmt,  1, tweet_id);                              // tid
  sqlite3_bind_int(stmt,  2, user_id);                               // writer_id
  sqlite3_bind_text(stmt, 3, text.c_str(), -1, SQLITE_STATIC);       // text
  sqlite3_bind_text(stmt, 4, this->_getDate(), -1, SQLITE_STATIC);   // tdate
  sqlite3_bind_text(stmt, 5, this->_getTime(), -1, SQLITE_STATIC);   // ttime

  // Execute the query.
  if (sqlite3_step(stmt) == SQLITE_ROW) {
      post_added = true;
  }
  sqlite3_finalize(stmt);
  
  return post_added;
}

/**
 * @brief Adds a reply tweet to the tweets table in the database.
 *
 * @param user_id The ID of the user creating the reply.
 * @param reply_tweet_id The ID of the tweet being replied to.
 * @param text The text content of the reply.
 * @return true if the reply was successfully added; false otherwise.
 */
bool Pond::addReply(const int32_t& user_id, const int32_t& reply_tweet_id, const std::string& text) {
  bool reply_added = false;

  const char* query = 
    "INSERT INTO tweets (tid, writer_id, text, tdate, ttime, replyto_tid) "
    "VALUES (?, ?, ?, ?, ?, ?)";

  // Prepare the SQL statement.
  sqlite3_stmt* stmt;
  if (sqlite3_prepare_v2(this->_db, query, -1, &stmt, nullptr) != SQLITE_OK) {
    sqlite3_finalize(stmt);
    return false;
  }

  // int32_t new_tid = generateUniqueTweetID();
  int32_t new_tid = 1; // Temporary ._. Fix LTR


  // Bind parameters to prevent SQL injection
  sqlite3_bind_int(stmt,  1, new_tid);                               // tid;
  sqlite3_bind_int(stmt,  2, user_id);                               // writer_id
  sqlite3_bind_text(stmt, 3, text.c_str(), -1, SQLITE_STATIC);       // text
  sqlite3_bind_text(stmt, 4, this->_getDate(), -1, SQLITE_STATIC);   // tdate
  sqlite3_bind_text(stmt, 5, this->_getDate(), -1, SQLITE_STATIC);   // ttime
  sqlite3_bind_int(stmt,  6, reply_tweet_id);                        // replyto_tid

  // Execute the query.
  if (sqlite3_step(stmt) == SQLITE_DONE) {
      reply_added = true;
  }
  sqlite3_finalize(stmt);
  
  return reply_added;
}

/**
 * @brief Creates a new list for a user in the database.
 *
 * @param user_id The ID of the user who owns the list.
 * @param list_name The name of the new list.
 * @return true if the list was successfully created; false otherwise.
 */
bool Pond::createList(const int32_t& user_id, const std::string& list_name) {
  bool list_created = false;

  const char* query = 
    "INSERT INTO lists (owner_id, lname) "
    "VALUES (?, ?)";

  // Prepare the SQL statement.
  sqlite3_stmt* stmt;
  if (sqlite3_prepare_v2(this->_db, query, -1, &stmt, nullptr) != SQLITE_OK) {
    sqlite3_finalize(stmt);
    return false;
  }

  // Bind parameters to prevent SQL injection.
  sqlite3_bind_int(stmt, 1, user_id);                            // owner_id
  sqlite3_bind_text(stmt, 2, list_name.c_str(), -1, SQLITE_STATIC); // lname

  // Execute the query.
  if (sqlite3_step(stmt) == SQLITE_DONE) {
    list_created = true;
  }

  sqlite3_finalize(stmt);
  return list_created;
}

/**
 * @brief Adds a tweet to a list in the database.
 *
 * @param list_id The name of the list.
 * @param tweet_id The ID of the tweet to add to the list.
 * @param user_id The ID of the user who owns the list.
 * @return true if the tweet was successfully added to the list; false otherwise.
 */
bool Pond::addToList(const std::string& list_name, const int32_t& tweet_id, const int32_t& user_id) {
  bool added_to_list = false;

  // check for existence first
  if (!this->_listExists(list_name, user_id)) {
    return added_to_list;
  }

  const char* query = 
    "INSERT INTO include (owner_id, lname, tid) "
    "VALUES (?, ?, ?)";

  // Prepare the SQL statement.
  sqlite3_stmt* stmt;
  if (sqlite3_prepare_v2(this->_db, query, -1, &stmt, nullptr) != SQLITE_OK) {
    sqlite3_finalize(stmt);
    return false;
  }

  // Bind parameters to prevent SQL injection.
  sqlite3_bind_int(stmt, 1, user_id);                          // owner_id
  sqlite3_bind_text(stmt, 2, list_name.c_str(), -1, SQLITE_STATIC); // lname
  sqlite3_bind_int(stmt, 3, tweet_id);                         // tid

  // Execute the query.
  if (sqlite3_step(stmt) == SQLITE_DONE) {
    added_to_list = true;
  }

  sqlite3_finalize(stmt);
  return added_to_list;
}


/**
 * @brief Checks if the provided user ID and password are valid for login.
 *
 * @param user_id The user ID to check in the database.
 * @param password The password corresponding to the user ID.
 * @return true if the login credentials are valid; false otherwise.
 */
int32_t* Pond::checkLogin(const int32_t& user_id, const std::string& password) {
  int32_t* user_id_ptr = nullptr;

  const char* query = 
    "SELECT * "
    "FROM users "
    "WHERE usr = ? "
    "AND pwd = ?";
  
  // Prepare the SQL statement.
  sqlite3_stmt* stmt;
  if (sqlite3_prepare_v2(this->_db, query, -1, &stmt, nullptr) != SQLITE_OK) {
    sqlite3_finalize(stmt);
    return nullptr;
  }

  // Bind parameters to prevent SQL injection.
  sqlite3_bind_int(stmt, 1, user_id);                               // usr
  sqlite3_bind_text(stmt, 2, password.c_str(), -1, SQLITE_STATIC);  // pwd


  // Execute the query.
  if (sqlite3_step(stmt) == SQLITE_ROW) {
    int32_t retrieved_id = sqlite3_column_int(stmt, 0);
    user_id_ptr = new int32_t(retrieved_id);
  }
  sqlite3_finalize(stmt);
  
  return user_id_ptr;
}

std::string Pond::getUsername(const int32_t& user_id) {
  std::string username;

  const char* query = 
    "SELECT name "
    "FROM users "
    "WHERE usr = ?";

  sqlite3_stmt* stmt;
  if (sqlite3_prepare_v2(this->_db, query, -1, &stmt, nullptr) != SQLITE_OK) {
    sqlite3_finalize(stmt);
    return "";
  }

  sqlite3_bind_int(stmt, 1, user_id);

  if (sqlite3_step(stmt) == SQLITE_ROW) {
    const unsigned char* retrieved_username = sqlite3_column_text(stmt, 0);
    if (retrieved_username != nullptr) {
      username = reinterpret_cast<const char*>(retrieved_username);
    }
  }

  sqlite3_finalize(stmt);

  return username;
}

std::vector<std::string> Pond::getFeed(const int32_t& user_id) {
    std::vector<std::string> feed;

    const char* query = 
        "SELECT 'tweet' AS type, t1.tid, u1.name, t1.writer_id, t1.tdate AS date, t1.ttime AS time, t1.text "
        "FROM tweets t1 "
        "JOIN follows f1 ON t1.writer_id = f1.flwee "
        "JOIN users u1 ON t1.writer_id = u1.usr "
        "WHERE f1.flwer = ? "
        "UNION "
        "SELECT 'retweet' AS type, t2.tid, u2.name, r.retweeter_id AS writer_id, r.rdate AS date, t2.ttime AS time, t2.text "
        "FROM retweets r "
        "JOIN tweets t2 ON t2.tid = r.tid "
        "JOIN follows f2 ON r.retweeter_id = f2.flwee "
        "JOIN users u2 ON r.retweeter_id = u2.usr "
        "WHERE f2.flwer = ? AND r.spam = 0 "
        "ORDER BY date DESC, time DESC";

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(this->_db, query, -1, &stmt, nullptr) != SQLITE_OK) {
        sqlite3_finalize(stmt);
        return feed;
    }

    sqlite3_bind_int(stmt, 1, user_id);
    sqlite3_bind_int(stmt, 2, user_id);
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char* username = sqlite3_column_text(stmt, 2);  // Username of the tweet author
        const unsigned char* date = sqlite3_column_text(stmt, 4);      // Date of tweet/retweet
        const unsigned char* time = sqlite3_column_text(stmt, 5);      // Time of tweet/retweet
        const unsigned char* text = sqlite3_column_text(stmt, 6);      // Text of tweet/retweet

        std::ostringstream oss;
        oss << (username ? reinterpret_cast<const char*>(username) : "Unknown");
        oss << std::string(80 - oss.str().length(), ' '); 
        oss << (date ? reinterpret_cast<const char*>(date) : "Unknown")
            << " " << (time ? reinterpret_cast<const char*>(time) : "Unknown") << "\n\n";
        oss << (text ? reinterpret_cast<const char*>(text) : "") << "\n";

        feed.push_back(oss.str());
    }

    sqlite3_finalize(stmt);

    return feed;
}

/**
 * @brief Adds a follow relationship between two users.
 *
 * Inserts a new record into the "follows" table,
 * indicating that the user with ID `user_id` has started following
 * the user with ID `follow_id`. The current date is added as the
 * start date of the follow relationship.
 *
 * @param user_id The ID of the user who is following.
 * @param follow_id The ID of the user to be followed.
 * @return true if the follow was successfully added, false otherwise.
 */
bool Pond::follow(const int32_t& user_id, const int32_t& follow_id) {
  bool follow_added = false;

  const char* query = 
    "INSERT INTO follows (flwer, flwee, start_date) "
    "VALUES (?, ?, ?)";
  
  // Prepare the SQL statement.
  sqlite3_stmt* stmt;
  if (sqlite3_prepare_v2(this->_db, query, -1, &stmt, nullptr) != SQLITE_OK) {
    sqlite3_finalize(stmt);
    return false;
  }

  // Bind parameters to prevent SQL injection.
  sqlite3_bind_int(stmt, 1, user_id);                               // follower_id
  sqlite3_bind_int(stmt, 2, follow_id);                             // followee_id
  sqlite3_bind_text(stmt, 3, this->_getDate(), -1, SQLITE_STATIC);  // start_date


  // Execute the query.
  if (sqlite3_step(stmt) == SQLITE_DONE) {
    follow_added = true;
  }
  sqlite3_finalize(stmt);
  
  return follow_added;
}

/**
 * @brief Removes a follow relationship between two users.
 *
 * Deletes a record from the "follows" table,
 * indicating that the user with ID `user_id` has stopped
 * following the user with ID `follow_id`.
 *
 * @param user_id The ID of the user who is unfollowing.
 * @param follow_id The ID of the user to be unfollowed.
 * @return true if the unfollow was successful, false otherwise.
 */
bool Pond::unfollow(const int32_t& user_id, const int32_t& follow_id) {
  bool unfollowed = false;

  const char* query = 
    "DELETE FROM follows "
    "WHERE flwer = ? "
    "AND flwee = ?";
  
  // Prepare the SQL statement.
  sqlite3_stmt* stmt;
  if (sqlite3_prepare_v2(this->_db, query, -1, &stmt, nullptr) != SQLITE_OK) {
    sqlite3_finalize(stmt);
    return false;
  }

  // Bind parameters to prevent SQL injection.
  sqlite3_bind_int(stmt, 1, user_id);   // follower_id
  sqlite3_bind_int(stmt, 2, follow_id); // followee_id

  // Execute the query.
  if (sqlite3_step(stmt) == SQLITE_DONE) {
    unfollowed = true;
  }
  sqlite3_finalize(stmt);
  
  return unfollowed;
}

bool Pond::get_unique_user_id(int32_t& unique_id) {
  unique_id = 1;
  bool found = false;
  
  const char* query =
    "SELECT usr FROM users WHERE usr >= 0 ORDER BY usr ASC";
  
  sqlite3_stmt* stmt;
  if (sqlite3_prepare_v2(this->_db, query, -1, &stmt, nullptr) != SQLITE_OK) {
    sqlite3_finalize(stmt);
    return false;
  }
  
  while (sqlite3_step(stmt) == SQLITE_ROW) {
    int32_t current_id = sqlite3_column_int(stmt, 0);

    if (current_id == unique_id) {
      unique_id++;
    } else if (current_id > unique_id) {
      found = true;
      break;
    }
  }
  
  sqlite3_finalize(stmt);

  if (!found && unique_id > INT32_MAX) {
    unique_id = -1;
    const char* query_neg =
      "SELECT usr FROM users WHERE usr < 0 ORDER BY usr DESC";

    if (sqlite3_prepare_v2(this->_db, query_neg, -1, &stmt, nullptr) != SQLITE_OK) {
      sqlite3_finalize(stmt);
      return false;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
      int32_t current_id = sqlite3_column_int(stmt, 0);

      if (current_id == unique_id) {
        unique_id--;
      } else if (current_id < unique_id) {
        break;
      }
    }
    sqlite3_finalize(stmt);
  }
  
  return true;
}

/**
 * @brief Retrieves the current time in GMT as a formatted string (HH:MM:SS).
 *
 * @return A string representing the current time in "HH:MM:SS" format.
 */
char* Pond::_getTime() {
  std::time_t rn = std::time(nullptr);
  std::tm* gmt = std::gmtime(&rn);

  char* t = new char[9];
  std::strftime(t, sizeof(t), "%H:%M:%S", gmt);
  
  return t;
}

/**
 * @brief Retrieves the current date in GMT as a formatted string (YYYY-MM-DD).
 *
 * @return A string representing the current date in "YYYY-MM-DD" format.
 */
char* Pond::_getDate() {
  std::time_t rn = std::time(nullptr);
  std::tm* gmt = std::gmtime(&rn);

  char* t = new char[11];
  // yyyy-mm-dd
  std::strftime(t, sizeof(t), "%F", gmt);

  return t;
}

/**
 * @brief Checks if a list exists for a given user in the database.
 *
 * This function verifies the existence of a list identified by `list_name`
 * for a specific user identified by `user_id` within the `lists` table
 * of the database.
 *
 * @param list_name The name of the list to check for.
 * @param user_id The ID of the user who owns the list.
 * @return True if the list exists for the specified user, false otherwise.
 *
 * The function prepares and executes an SQL query to determine whether a row
 * exists in the `lists` table with the given `owner_id` and `lname` values.
 * Parameter binding is used to prevent SQL injection vulnerabilities.
 *
 * @note If there is an error preparing the SQL statement, the function will
 *       finalize the statement and return false.
 */
bool Pond::_listExists(const std::string &list_name, const int32_t &user_id) {
  bool exists = false;

  const char* query = "SELECT 1 FROM lists WHERE owner_id = ? AND lname = ?";

  sqlite3_stmt* stmt;
  if (sqlite3_prepare_v2(this->_db, query, -1, &stmt, nullptr) != SQLITE_OK) {
    sqlite3_finalize(stmt);
    return false;
  }

  // Bind parameters to prevent SQL injection.
  sqlite3_bind_int(stmt, 1, user_id);                                     // owner_id
  sqlite3_bind_text(stmt, 2, list_name.c_str(), -1, SQLITE_STATIC);       // lname

  // Execute the query.
  exists = sqlite3_step(stmt) == SQLITE_ROW;
  sqlite3_finalize(stmt);

  if (!exists) {
    return false;
  }
  else {
    return true;
  }
}