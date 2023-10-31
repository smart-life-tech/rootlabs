exports = async function (payload) {
    const atlas = context.services.get("mongodb-atlas");
    const coll = atlas.db("iot").collection("readings");
    var news = "";

    try {
        if (payload.body) {
            const data = EJSON.parse(payload.body.text());
            const macAddress = Object.keys(data)[0]; // Extract the MAC address from the payload
            console.log('MAC Address:', macAddress);
            console.log('Data:', data);

            const filter = { [macAddress]: { $exists: true } }; // Filter to match documents with the MAC address

            // Check if a document with the MAC address exists
            const existingData = await coll.findOne(filter);

            if (existingData) {
                // Document with the MAC address exists, update it
                const update = {
                    $set: {}
                };
                // Add your update logic here

                // Update the "ts" field with the current timestamp
                update.$set[`${macAddress}.ts`] = new Date();

                // Perform the update operation
                coll.updateOne(filter, update, { upsert: false });
            } else {
                // Document with the MAC address does not exist, insert a new document
                const newDocument = {
                    [macAddress]: {
                        ts: new Date(),
                        // Add your other data fields here
                    }
                };
                // Insert the new document
                coll.insertOne(newDocument);
            }

            // Your code to construct the "news" string here

        }

        return news;
    } catch (e) {
        console.log("Error updating/inserting doc: " + e);
        return e.message();
    }
};
