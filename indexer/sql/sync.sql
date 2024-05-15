CREATE PROCEDURE synchronize_with_secondary() AS
$$
    TRUNCATE TABLE resources_lexems;
    TRUNCATE TABLE champion_lists;
    TRUNCATE TABLE lexems CASCADE;
    TRUNCATE TABLE resources CASCADE;
    TRUNCATE TABLE resources_headers CASCADE;
    TRUNCATE TABLE domains CASCADE;
    TRUNCATE TABLE methods CASCADE;

    INSERT INTO methods             SELECT * FROM methods_sec;   
    INSERT INTO domains             SELECT * FROM domains_sec;
    INSERT INTO resources_headers   SELECT * FROM resources_headers_sec;
    INSERT INTO resources           SELECT * FROM resources_sec;
    INSERT INTO lexems              SELECT * FROM lexems_sec;
    INSERT INTO resources_lexems    SELECT * FROM resources_lexems_sec;
    INSERT INTO champion_lists      SELECT * FROM champion_lists_sec;

    TRUNCATE TABLE resources_adjacency_sec;
    TRUNCATE TABLE temporary_resource_ranks_sec;
    TRUNCATE TABLE resources_lexems_sec;
    TRUNCATE TABLE champion_lists_sec;
    TRUNCATE TABLE lexems_sec RESTART IDENTITY CASCADE;
    TRUNCATE TABLE resources_sec RESTART IDENTITY CASCADE;
    TRUNCATE TABLE resources_headers_sec RESTART IDENTITY CASCADE;
    TRUNCATE TABLE domains_sec RESTART IDENTITY CASCADE;
    TRUNCATE TABLE methods_sec RESTART IDENTITY CASCADE;  
$$ LANGUAGE SQL;