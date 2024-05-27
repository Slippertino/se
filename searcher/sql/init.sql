CREATE VIEW resources_named_view AS
	SELECT 
		rs.id,
		ms.name AS method, 
		ds.name AS domain, 
		rs.name AS name,
		rs.title,
		rs.compression_type,
		rs.content,
		rs.size,
		rs.rank
	FROM resources AS rs
	INNER JOIN resources_headers AS rhs ON rhs.id = rs.resource_header_id
	INNER JOIN domains AS ds ON ds.id = rhs.domain_id
	INNER JOIN methods AS ms ON ms.id = rhs.method_id;

CREATE TYPE public.new_lexem_entry_type AS (
    name character varying(300),
    lang character varying(20),
    wlf real
);

CREATE TYPE public.lexem_type AS (
    lexem_id integer,
    rank real
);

CREATE TYPE public.resource_rank AS (
    dynamic_rank real,
    static_rank real
);

CREATE TYPE public.ranked_resource AS (
    id bigint,
    rank real
);

CREATE TYPE public.resource_info AS (
    url text,
    title text,
    relevance real
);

CREATE FUNCTION get_existing_lexems(entries new_lexem_entry_type[])
RETURNS TABLE(lexem_id int, lexem_rank real) AS 
$$
DECLARE res lexem_type[];
DECLARE len real;
BEGIN
	SELECT ARRAY(
		SELECT row(l.id, nl.wlf * LOG((SELECT COUNT(*) FROM resources)::real / l.df))::lexem_type
		FROM lexems AS l
		INNER JOIN UNNEST(entries) AS nl ON nl.name = l.name AND nl.lang = l.lang 
	) INTO res;

	SELECT SQRT(SUM(POWER(rank, 2)))
	FROM UNNEST(res) INTO len;

	RETURN QUERY
	SELECT r.lexem_id, r.rank / len
	FROM UNNEST(res) AS r;
END
$$ LANGUAGE plpgsql;

CREATE FUNCTION get_champ_docs(lexems int[])
RETURNS bigint[] AS 
$$
	SELECT ARRAY_AGG(res_id)
	FROM (
		SELECT DISTINCT UNNEST(resources_list)
		FROM champion_lists AS cl
		WHERE lexem_id IN (SELECT UNNEST(lexems))
	) AS r(res_id);
$$ LANGUAGE SQL;

CREATE FUNCTION get_resources_info(resources ranked_resource[])
RETURNS resource_info[] AS
$$
    SELECT ARRAY(
        SELECT row(rnv.method || '://'::text || rnv.domain || rnv.name, rnv.title, rs.rank)::resource_info
        FROM resources_named_view AS rnv
        INNER JOIN (SELECT id, rank FROM UNNEST(resources)) AS rs ON rs.id = rnv.id
		ORDER BY rs.rank DESC
    );
$$ LANGUAGE SQL;

CREATE FUNCTION estimate_resource_rank(resource_id bigint, lexems lexem_type[])
RETURNS resource_rank AS
$$
DECLARE resource_len real;
DECLARE query_len real;
DECLARE scalar_mult real;
BEGIN
	SELECT SQRT(SUM(POWER(rl.rank, 2)))
	FROM resources_lexems AS rl
	WHERE res_id = resource_id
	INTO resource_len;

	SELECT SQRT(SUM(POWER(l.rank, 2)))
	FROM UNNEST(lexems) AS l
	INTO query_len;

    SELECT SUM(rl.rank * l.rank) / (resource_len * query_len)
    FROM resources_lexems AS rl
    INNER JOIN (SELECT lexem_id, rank FROM UNNEST(lexems)) AS l ON rl.lexem_id = l.lexem_id
    WHERE res_id = resource_id
    INTO scalar_mult;

	RETURN(
	    SELECT row(scalar_mult, r.rank)::resource_rank
	    FROM resources AS r
	    WHERE r.id = resource_id
	);
END
$$ LANGUAGE plpgsql;