# Idea

The main idea is to convert SQL queries to C++ native code and compile them into libraries, and run them.

SQL -> C++ -> Compiler -> Loadable library -> Execute.

For example this query:

	CREATE TABLE "asdf" (
		"id" integer
	);

Produces this code:

	struct asdf_record
	{
		int id;
	};

	std::list<asdf_record> asdf_collection;

And this query:

	SELECT "id" FROM "asdf" WHERE "id" = 5 LIMIT 1

Might produce:

	int limit = 0;
	for (asdf_record record : asdf_collection)
	{
		if (record.id == 5)
		{
			// store record into result data set
			if (++limit == 1)
				break;
		}
	}

All these codes will be compiled into loadable libraries and executed.

# Really?

Maybe. I was bored.