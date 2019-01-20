using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Threading.Tasks;
using Microsoft.Azure.Documents.Client;
using Microsoft.Azure.Documents.Linq;


namespace opentheknock
{
    public partial class DBManager
    {
        static DBManager defaultInstance = new DBManager();
        const string accountURL = @"";
        const string accountKey = @"";
        const string databaseId = @"Users";
        const string collectionId = @"OTK";

        private Uri collectionLink = UriFactory.CreateDocumentCollectionUri(databaseId, collectionId);

        private DocumentClient client;

        private DBManager()
        {
            client = new DocumentClient(new System.Uri(accountURL), accountKey);
        }

        public static DBManager DefaultManager
        {
            get
            {
                return defaultInstance;
            }
            private set
            {
                defaultInstance = value;
            }
        }

        public Tuple<string, bool, Dictionary<int, string>> GetUsersChallenges(int userId)
        {
            try
            {
                var challengesMap = new Dictionary<int, string>();

                UserItem user =
                    client.CreateDocumentQuery<UserItem>(collectionLink,
                            new FeedOptions { EnableCrossPartitionQuery = true })
                        .Where(userItem => userItem.UserId == userId)
                        .AsEnumerable()
                        .FirstOrDefault();

                challengesMap.Add(1, user.Challenge1);
                challengesMap.Add(2, user.Challenge2);
                challengesMap.Add(3, user.Challenge3);

                return new Tuple<string, bool, Dictionary<int, string>> (user.Email, user.EmailNotifications, challengesMap);

            }
            catch (Exception e)
            {
                Console.Error.WriteLine(@"ERROR {0}", e.Message);
                return null;
            }
        }

        public async Task<UserItem> SaveData(UserItem userItem)
        {
            // TODO: Check if user item exists
            UserItem userExist = GetUserByUserId(userItem.UserId);

            if (userExist == null)
            {
                try
                {
                    var result = await client.CreateDocumentAsync(collectionLink, userItem);
                }
                catch (Exception e)
                {
                    Console.Error.WriteLine(@"ERROR {0}", e.Message);
                }
            }
            else
            {
                // Create new user with same id
                UserItem updatedUser = new UserItem();
                updatedUser.Id = userExist.Id;
                updatedUser.UserId = userExist.UserId;
                updatedUser.Email = userItem.Email;
                updatedUser.EmailNotifications = userItem.EmailNotifications;
                updatedUser.Challenge1 = userItem.Challenge1;
                updatedUser.Challenge2 = userItem.Challenge2;
                updatedUser.Challenge3 = userItem.Challenge3;
                try
                {
                    var result = await client.UpsertDocumentAsync(collectionLink, updatedUser);
                }
                catch (Exception e)
                {
                    Console.Error.WriteLine(@"ERROR {0}", e.Message);
                }

            }


            return userExist;
        }

        public UserItem GetUserByUserId(int userId)
        {
            UserItem user = null;
            try
            {
                user =
                    client.CreateDocumentQuery<UserItem>(collectionLink,
                            new FeedOptions { EnableCrossPartitionQuery = true })
                        .Where(userItem => userItem.UserId == userId)
                        .AsEnumerable()
                        .FirstOrDefault();
            }
            catch (Exception e)
            {
                Console.Error.WriteLine(@"ERROR {0}", e.Message);
            }
            return user;
        }
    }
}
