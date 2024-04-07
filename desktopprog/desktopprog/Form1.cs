using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using Firebase.Database;
using Firebase.Database.Query;

namespace desktopprog
{
    public partial class Form1 : Form
    {
        private readonly FirebaseClient firebase;

        public Form1()
        {
            InitializeComponent();
            label3.Visible = false;

            firebase = new FirebaseClient("URL BAZE PODATAKA");

            // Start listening for changes in the database
            ListenToDatabase();


        }
        private async void ListenToDatabase()
        {
            var dataRef = firebase.Child("data/indicator"); // Replace "yourDataNode" with the actual node in your database

            while (true)
            {
                var data = await dataRef.OnceSingleAsync<float>();

                UpdateLabel(data);

                await Task.Delay(1000);
            }
        }
        private void UpdateLabel(float data)
        {
            // Update label text
            label1.Text = $"{data}";

            // Change label color based on a certain condition
            label1.ForeColor = data < 10 ? Color.Red : Color.Green;


            if(data < 10)
            {
                label3.Visible = true;
            }
            else
            {
                label3.Visible = false;
            }
        }


    }
}

