using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Net.Mail;
using System.Text.RegularExpressions;

namespace ThorDataAccessLayer
{
    public class Emailer
    {
        #region PublicMethod

        public static string sendEmail(string receiver, string sender, string emailSubject, string emailMessage, string host)
        {
            string errorMessage;
            try
            {
                //check if all feilds are available
                bool valid = validateInputs(receiver, sender, emailSubject, emailMessage, host, out errorMessage);
                if (valid == true)
                {

                    // Check if receiver email id is Correct
                    bool isReceiverValid = validateEmailAddress(receiver);
                    if (isReceiverValid == false)
                        return "Error : Invalid Email Address For Receiver : " + receiver;

                    bool isSenderValid = validateEmailAddress(sender);
                    if (isSenderValid == false)
                        return "Error : Invalid Email Address For Sender : " + sender;

                    //Sreate Message
                    MailMessage message = new MailMessage(sender, receiver, emailSubject, emailMessage);

                    //Create SMPT Client 
                    //Passed Name of internet provider's default SMTP mail server
                    SmtpClient client = new SmtpClient(host);
                    client.UseDefaultCredentials = true;

                    //Send Message
                    client.Send(message);
                    return  DateTime.Now.ToString();
                }
                else
                {
                    return errorMessage; 
                }

            }
            catch (Exception ex)
            {
                return "Error : " + ex.Message;
            }
        }

        #endregion

        # region privateMethods

        private static bool validateEmailAddress(string emailId)
        {
            try
            {
                string TextToValidate = emailId;
                Regex expression = new Regex(@"\w+@[a-zA-Z_]+?\.[a-zA-Z]{2,3}");
                // test email address with expression
                if (expression.IsMatch(TextToValidate))
                    return true;
                else
                    return false;
            }
            catch (Exception)
            {
                return false;
            }
        }

        private static bool validateInputs(string receiver, string sender, string emailSubject, string emailMessage, string host, out string message)
        {
            receiver = receiver.Trim();
            sender = sender.Trim();
            emailMessage = emailMessage.Trim();
            emailSubject = emailSubject.Trim();
            host = host.Trim();

            bool valid = false;
            if (String.IsNullOrEmpty(receiver))
            {
                message = "Error : Receiver Field Empty.";
                valid = false;
            }
            else if (String.IsNullOrEmpty(sender))
            {
                message = "Error : Sender Field Empty.";
                valid = false;
            }
            else if (String.IsNullOrEmpty(emailSubject))
            {
                message = "Error : Email Subject Field Empty.";
                valid = false;
            }
            else if (String.IsNullOrEmpty(emailMessage))
            {
                message = "Error : Email Message Field Empty.";
                valid = false;
            }
            else if (String.IsNullOrEmpty(host))
            {
                message = "Error : Host Unavailable.";
                valid = false;
            }
            else
            {
                message = "Inputs Validation Passed";
                valid = true;
            }
            
            return valid;
        }

        #endregion

    }
}
