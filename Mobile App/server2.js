const express = require('express');
const bodyParser = require('body-parser');
const mysql = require('mysql');
const crypto = require('crypto');
const http = require('http');

// Create an instance of express
const app = express();

// Use body-parser middleware to parse JSON requests
app.use(bodyParser.json());



// Function to generate a random string of given length
function generateRandomString(length) {
  return crypto.randomBytes(Math.ceil(length / 2))
    .toString('hex') // Convert to hexadecimal format
    .slice(0, length); // Trim to desired length
}


// Create a MySQL connection
const db = mysql.createConnection({
  host: 'localhost',     // Replace with your MySQL server host
  user: 'himeth',  // Replace with your MySQL username
  password: 'HimWsql@#135',  // Replace with your MySQL password
  database: 'smartglow'   // Replace with your MySQL database name
});

// Connect to the database
db.connect((err) => {
  if (err) {
    console.error('Error connecting to the database:', err.stack);
    return;
  }
  console.log('Connected to the database.');
});

// Define a POST route to handle incoming JSON data
app.post('/save', (req, res) => {
  const { username, password, email } = req.body;

  // Validate the input data
  if (!username || !password || !email) {
    return res.status(400).send('Missing required fields');
  }
  // Generate a random 50-character string
  const randomString = generateRandomString(50)

  // Insert the data into the database
  const sql = 'INSERT INTO userMaster (username, password, topic, email) VALUES (?, ?, ?, ?)';
  db.query(sql, [username, password,randomString, email], (err, result) => {
    if (err) {
      console.error('Error inserting data into the database:', err.stack);
      return res.status(500).send('Error inserting data into the database');
    };
    res.status(200).send(randomString);

})

});

////////////////////////////////////////////////
app.post('/login', (req, res) => {

    const { username , password} = req.body;
    // Validate the input data
    if (!username || !password) {
      return res.status(400).send('Missing bulbs');
    }
  
    const sql = 'SELECT password,id,topic FROM userMaster WHERE username = ?';
    db.query(sql, [username], (err, result) => {
    if (err) {
      console.error('Error querying the database:', err.stack);
      return res.status(500).send('Error querying the database');
    }
  
    if (result.length === 0) {
      return res.status(401).send('Invalid username or password');
    }
  
    const storedHashedPassword = result[0].password;
    const userid = result[0].id
    const usertopic = result[0].topic
  
    // Compare the hashed password sent from the client with the stored hashed password
    if (password === storedHashedPassword) {
  
      const sql2 = 'SELECT bulbid, bulbname, description FROM bulbinfo WHERE userid = ?';
      db.query(sql2, [userid], (err, bulbresult) => {
      if (err) {
        console.error('Error getting bulbs', err.stack);
        return res.status(500).send('Error querying the database');
      }

      const response = {
        bulbs: bulbresult,
        topic: usertopic
      };

      res.status(200).send(response);
    })
  
    } else {
      res.status(401).send('Invalid');
    }
  });
  
  });

//////////////////////////////////////////////
app.post('/add', (req, res) => {

  const { bulbindex , topic , bulbname , description } = req.body;
  // Validate the input data
  if (!bulbindex || !topic || !bulbname || !description) {
    return res.status(400).send('Missing bulbs');
  }
  const sql = 'SELECT id FROM userMaster WHERE topic = ?';
  db.query(sql, [topic], (err, result) => {
    if (err) {
      console.error('Error getting userid:', err.stack);
      return res.status(500).send('Error getting userid');
    };

    if (result.length === 0) {
      // No user found for the provided topic
      return res.status(404).send('No user found for the provided topic');
    }

    const query_userid = result[0].id;

    const sql2 = 'INSERT INTO bulbinfo (userid, bulbid, bulbname, description) VALUES(?,?,?,?)';
    db.query(sql2, [query_userid,bulbindex,bulbname,description], (err, result) => {
      if (err) {
        console.error('Error inserting bulbinfo into the database:', err.stack);
        return res.status(500).send('Error inserting bulbinfo into the database');
      };
    res.status(200).send(bulbindex.toString());
    });

});
});

///////////////////////////////////////////////////////////////////////////////////////////
app.post('/rename', (req, res) => {

  const { bulbindex , topic , bulbname , description } = req.body;
  // Validate the input data
  if (!bulbindex || !topic || !bulbname || !description) {
    return res.status(400).send('Missing bulbs');
  }
  const sql = 'SELECT id FROM userMaster WHERE topic = ?';
  db.query(sql, [topic], (err, result) => {
    if (err) {
      console.error('Error getting userid:', err.stack);
      return res.status(500).send('Error getting userid');
    };

    if (result.length === 0) {
      // No user found for the provided topic
      return res.status(404).send('No user found for the provided topic');
    }

    const query_userid = result[0].id;

    const sql2 = 'UPDATE bulbinfo SET bulbname = ?, description = ? WHERE (userid = ? AND bulbid = ?)';
    db.query(sql2, [bulbname,description,query_userid,bulbindex], (err, result) => {
      if (err) {
        console.error('Error updating bulbinfo into the database:', err.stack);
        return res.status(500).send('Error updating bulbinfo into the database');
      };
    res.status(200).send(bulbindex.toString());
    });

});
});

/////////////////////////////////////////////////////////////////////////////

app.post('/remove', (req, res) => {

  const { bulbindex , topic } = req.body;
  // Validate the input data
  if (!bulbindex || !topic) {
    return res.status(400).send('Missing bulbs');
  }
  const sql = 'SELECT id FROM userMaster WHERE topic = ?';
  db.query(sql, [topic], (err, result) => {
    if (err) {
      console.error('Error getting userid:', err.stack);
      return res.status(500).send('Error getting userid');
    };

    if (result.length === 0) {
      // No user found for the provided topic
      return res.status(404).send('No user found for the provided topic');
    }

    const query_userid = result[0].id;

    const sql2 = 'DELETE FROM bulbinfo WHERE (userid = ? AND bulbid = ?)';
    db.query(sql2, [query_userid,bulbindex], (err, result) => {
      if (err) {
        console.error('Error updating bulbinfo into the database:', err.stack);
        return res.status(500).send('Error updating bulbinfo into the database');
      };
    res.status(200).send(bulbindex.toString());
    });

});
});




// Start the server
const PORT = process.env.PORT || 5000;
app.listen(PORT, () => {
  console.log(`Server is running on port ${PORT}`);
});