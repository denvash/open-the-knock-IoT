using IoTHubTrigger = Microsoft.Azure.WebJobs.ServiceBus.EventHubTriggerAttribute;

using Microsoft.Azure.WebJobs;
using Microsoft.Azure.WebJobs.Host;
using Microsoft.Azure.EventHubs;
using System.Text;
using System;
using System.Net.Http;
using Newtonsoft.Json;
using System.Threading.Tasks;
using Microsoft.Azure.Devices;
using opentheknock;
using System.Collections.Generic;
using System.Linq;
using System.Text.RegularExpressions;

namespace otkvisualfunction
{
    public static class generator
    {
        private static HttpClient client = new HttpClient();
        private static ServiceClient s_serviceClient;

        [FunctionName("generator")]
        public static void Run([IoTHubTrigger("messages/events", Connection = "IoTEventTrigger")]EventData message, TraceWriter log)
        {
            /* retrieve user id */
            var bytesAsString = Encoding.UTF8.GetString(message.Body.Array);
            var person = JsonConvert.DeserializeObject<otkKnockMessage>(bytesAsString);
            log.Info($"C# IoT Hub trigger function processed a message: {Encoding.UTF8.GetString(message.Body.Array)}");
            log.Info($"personId: {person.userId}");

            /* retrieve user personal properties */
            Dictionary<int, string> userChallenges;
            DBManager dbClient = DBManager.DefaultManager;
            try {
                userChallenges = dbClient.GetUsersChallenges(person.userId);
                string passwordChallengeStr = userChallenges[1];
                string matrixChallengeStr = userChallenges[2];
                string menuChallengeStr = userChallenges[3];
                //log.Info($"challenge 1: {passwordChallengeStr}, challenge 2: {matrixChallengeStr}");
            } catch (Exception e)
            {
                log.Info($"db access error: {e}");
                return;
            }

            /* generate the challenge */
            otkKnockChallenge myChallenge = new otkKnockChallenge(userChallenges);
            string challengeJsonPayload = JsonConvert.SerializeObject(myChallenge);
            log.Info(challengeJsonPayload);
            /* send challenge properties to device */
            s_serviceClient = ServiceClient.CreateFromConnectionString("HostName=otk-iot.azure-devices.net;SharedAccessKeyName=iothubowner;SharedAccessKey=o+wrTuFd7yoFi/DMjJXKkNWXwlh04EiLkblShFTN9sk=");
            //SendCloudToDeviceMessageAsync().Wait();
            InvokeMethod("startChallenge", challengeJsonPayload).Wait();
        }

        // Connection string for your IoT Hub
        // az iot hub show-connection-string --hub-name {your iot hub name}
        //private readonly static string s_connectionString = "IoTServiceConnectionString";

        private async static Task SendCloudToDeviceMessageAsync()
        {
            var commandMessage = new Message(Encoding.ASCII.GetBytes("Cloud to device message."));
            await s_serviceClient.SendAsync("myKnockDevice", commandMessage);
        }

        private static async Task InvokeMethod(string methodName, string payloadJson)
        {
            Console.WriteLine("InvokeMethod called with payload:");
            Console.WriteLine(payloadJson);
            var methodInvocation = new CloudToDeviceMethod(methodName) { ResponseTimeout = TimeSpan.FromSeconds(30) };
            methodInvocation.SetPayloadJson(payloadJson);

            // Invoke the direct method asynchronously and get the response from the simulated device.
            var response = await s_serviceClient.InvokeDeviceMethodAsync("myKnockDevice", methodInvocation);

            Console.WriteLine("Response status: {0}, payload:", response.Status);
            Console.WriteLine(response.GetPayloadAsJson());
        }
    }

    public struct otkKnockMessage
    {
        public string deviceId;
        public int messageId;
        public string azureRoute;
        public int userId;
    }

    public class otkKnockChallenge
    {
        public MatrixChallenge matrixChallenge;
        public PasswordChallenge passwordChallenge;
        public MenuChallenge menuChallenge;

        public otkKnockChallenge()
        {
            //challengeNo = 1;
            //challengeSequence = "2,3";
        }

        public otkKnockChallenge(Dictionary<int, string> challenges)
        {
            passwordChallenge = new PasswordChallenge(challenges[1]);
            matrixChallenge = new MatrixChallenge(challenges[2]);
            menuChallenge = new MenuChallenge(challenges[3]);
        }

        /* temporary */
        //public int challengeNo;
        //public string challengeSequence;

        /* future implementation */
        //matrixChallenge matrixChallenge;
        //menuChallenge menuChallenge;
        //passwordChallenge passwordChallenge;    
    }

    public class MatrixChallenge
    {
        private const int solutionLength = Constants.solutionLength;
        private const int matrixSize = 3;
        private const int passLength = 9;
        public string[] challengeMatrices = new string[solutionLength];
        public int[] challengeSequence = new int[solutionLength];

        public MatrixChallenge(string matrix)
        {
            for (int i = 0; i < solutionLength; ++i)
            {
                Random random = new Random();
                const string chars = "01";
                challengeMatrices[i] = new string(Enumerable.Repeat(chars, passLength).Select(s => s[random.Next(s.Length)]).ToArray());
                challengeSequence[i] = generateSequence(matrix, challengeMatrices[i]);
            }
        }

        public int generateSequence(string orig, string generated)
        {
            int res = 0;
            for (int i = 0; i < generated.Length; i++)
            {
                if (generated[i] == '1' && orig[i] == '1')
                {
                    res++;
                }
            }
            return res % 5 + 1;
        }

        public static MatrixChallenge generate(string matrix)
        {
            MatrixChallenge myMatrix = new MatrixChallenge(matrix);
            return myMatrix;
        }
    }

    public class PasswordChallenge
    {
        private const int solutionLength = Constants.solutionLength;
        private const int passLength = 5;
        public string[] challengePasswords = new string[solutionLength];
        public int[] challengeSequence = new int[solutionLength];

        public PasswordChallenge(string password) {
            Random random = new Random();

            //increases chances of matches with adding password to list
            string chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789" + password.ToUpper() + password.ToUpper();
            chars.Replace('O', 'o');
            for(int i = 0; i < solutionLength; ++i) {
                challengePasswords[i] = new string (Enumerable.Repeat(chars, passLength).Select(s => s[random.Next(s.Length)]).ToArray());
                challengeSequence[i] = generateSequence(password, challengePasswords[i]);

                challengePasswords[i].Replace("0", "0.");
            }
        }

        public int generateSequence(string orig, string generated)
        {
            int res = 0;
            for (int i = 0; i < generated.Length; i++)
            {
                if (orig.ToLower().Contains(generated.ToLower()[i]))
                {
                    res++;
                }
            }
            return res % 5 + 1;
        }
   

    }


    public class MenuChallenge
    {
        private const int solutionLength = Constants.solutionLength;
        //private const int passLength = 9;
        private const int challengeLength = 5;
        public string[] challengeMenu = new string[challengeLength];
        public int[] challengeSequence = new int[solutionLength];


        public MenuChallenge(string menu)
        { 

            Random rnd = new Random();
            string splitter = @" \| ";

            string[] selectedItems = Regex.Split(menu, splitter);
            selectedItems = selectedItems.Where(x => !string.IsNullOrEmpty(x)).ToArray();
            string[] nonSelectedItems = Constants.MenuItems.Except(selectedItems).ToArray();
            //string[] MyRandomItemsArray = Constants.MenuItems.OrderBy(x => rnd.Next()).ToArray();

            //randomize nonSelectedItems
            nonSelectedItems = nonSelectedItems.OrderBy(x => rnd.Next()).ToArray();
            //randomize selectedItems
            selectedItems = selectedItems.OrderBy(x => rnd.Next()).ToArray();

            Array.Copy(nonSelectedItems, 0, challengeMenu, 0, challengeLength - solutionLength);
            Array.Copy(selectedItems, 0, challengeMenu, challengeLength - solutionLength, solutionLength);

            //randomize myRandomArray
            challengeMenu = challengeMenu.OrderBy(x => rnd.Next()).ToArray();
            int j = 0;
            for(int i = 0; i < challengeLength; i++)
            {
                int price = rnd.Next(1, Constants.menuItemMaxPrice);
                if(selectedItems.Contains(challengeMenu[i]))
                {
                    challengeSequence[j++] = price % 5 + 1;
                }
                int numOfDots = Constants.menuItemLength - challengeMenu[i].Length;
                const string chars = ".";
                string dotSequence = new string(Enumerable.Repeat(chars, numOfDots).Select(s => s[rnd.Next(s.Length)]).ToArray());
                challengeMenu[i] = challengeMenu[i] + dotSequence + price + "$";
            }
        }
    }
}