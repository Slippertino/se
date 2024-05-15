CREATE TABLE IF NOT EXISTS public.methods_sec
(
    id serial NOT NULL,
    name character varying(20) COLLATE pg_catalog."default" NOT NULL,
    CONSTRAINT pk_method_sec_id PRIMARY KEY (id),
    CONSTRAINT uk_method_sec_name UNIQUE (name)
);
    
CREATE TABLE IF NOT EXISTS public.domains_sec
(
    id serial NOT NULL,
    name character varying(50) COLLATE pg_catalog."default" NOT NULL,
    CONSTRAINT pk_domain_sec_id PRIMARY KEY (id),
    CONSTRAINT uk_domain_sec_name UNIQUE (name)
);

CREATE TABLE IF NOT EXISTS public.resources_headers_sec
(
    id serial NOT NULL,
	method_id serial NOT NULL,
    domain_id serial NOT NULL,
    CONSTRAINT pk_resources_headers_sec_id PRIMARY KEY (id),
	CONSTRAINT uk_resources_headers_sec_id UNIQUE(method_id, domain_id),
	CONSTRAINT fk_resources_headers_sec_method FOREIGN KEY (method_id)
        REFERENCES public.methods_sec (id) MATCH SIMPLE
        ON UPDATE CASCADE
        ON DELETE CASCADE,
    CONSTRAINT fk_resources_headers_sec_domain FOREIGN KEY (domain_id)
        REFERENCES public.domains_sec (id) MATCH SIMPLE
        ON UPDATE CASCADE
        ON DELETE CASCADE
);

CREATE TABLE IF NOT EXISTS public.resources_sec
(
	id bigserial NOT NULL,
	resource_header_id serial NOT NULL,
    name character varying(500) COLLATE pg_catalog."default" NOT NULL,
	title text NOT NULL,
	compression_type character varying(10) DEFAULT '',
	content bytea NOT NULL,
	size int NOT NULL,
	rank real NOT NULL DEFAULT 0.0,
    CONSTRAINT pk_resource_sec PRIMARY KEY (id),
	CONSTRAINT uk_resource_sec UNIQUE(resource_header_id, name),
    CONSTRAINT fk_resource_sec_resources_headers_sec FOREIGN KEY (resource_header_id)
        REFERENCES public.resources_headers_sec (id) MATCH SIMPLE
        ON UPDATE CASCADE
        ON DELETE CASCADE
);

CREATE TABLE IF NOT EXISTS public.resources_adjacency_sec
(
	id_from bigserial NOT NULL,
	id_to bigserial NOT NULL,
	probability real NOT NULL DEFAULT 0,
	CONSTRAINT pk_resources_adjacency_sec PRIMARY KEY(id_from, id_to),
	CONSTRAINT fk_resources_adjacency_sec_from_reources_sec FOREIGN KEY(id_from)
	    REFERENCES public.resources_sec (id) MATCH SIMPLE
        ON UPDATE CASCADE
        ON DELETE CASCADE,
	CONSTRAINT fk_resources_adjacency_sec_to_reources_sec FOREIGN KEY(id_to)
	    REFERENCES public.resources_sec (id) MATCH SIMPLE
        ON UPDATE CASCADE
        ON DELETE CASCADE
);

CREATE TABLE IF NOT EXISTS public.lexems_sec
(
	id serial NOT NULL,
	name character varying(300) NOT NULL,
	lang character varying(20) NOT NULL,
	df bigint NOT NULL DEFAULT 0,
	CONSTRAINT pk_lexems_sec PRIMARY KEY(id),
	CONSTRAINT uk_lexems_sec UNIQUE(name, lang)
);

CREATE TABLE IF NOT EXISTS public.resources_lexems_sec
(
	res_id bigserial NOT NULL,
	lexem_id serial NOT NULL,
    wlf real NOT NULL,
	rank real,
	CONSTRAINT pk_resources_lexems_sec PRIMARY KEY(res_id, lexem_id),
	CONSTRAINT fk_resources_lexems_sec_reources_sec FOREIGN KEY(res_id)
	    REFERENCES public.resources_sec (id) MATCH SIMPLE
        ON UPDATE CASCADE
        ON DELETE CASCADE,
	CONSTRAINT fk_resources_lexems_sec_lexems_sec FOREIGN KEY(lexem_id)
	    REFERENCES public.lexems_sec (id) MATCH SIMPLE
        ON UPDATE CASCADE
        ON DELETE CASCADE
);

CREATE TABLE IF NOT EXISTS public.champion_lists_sec
(
	lexem_id serial NOT NULL,
	resources_list bigint[] NOT NULL,
	CONSTRAINT pk_champion_lists_sec PRIMARY KEY(lexem_id),
	CONSTRAINT fk_champion_lists_sec_lexems_sec FOREIGN KEY(lexem_id)
	    REFERENCES public.lexems_sec (id) MATCH SIMPLE
        ON UPDATE CASCADE
        ON DELETE CASCADE
);

CREATE TABLE IF NOT EXISTS public.temporary_resource_ranks_sec
(
	res_id bigint NOT NULL,
	rank real NOT NULL DEFAULT 0
);

CREATE VIEW resources_sec_named_view AS
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
	FROM resources_sec AS rs
	INNER JOIN resources_headers_sec AS rhs ON rhs.id = rs.resource_header_id
	INNER JOIN domains_sec AS ds ON ds.id = rhs.domain_id
	INNER JOIN methods_sec AS ms ON ms.id = rhs.method_id;

CREATE TYPE public.new_lexem_type AS (
	name character varying(300),
	lang character varying(20)
);

CREATE TYPE public.new_lexem_entry_type AS (
    name character varying(300),
    lang character varying(20),
    wlf real
);

CREATE FUNCTION get_lexem_id_by_number(num int) 
RETURNS int AS
$$
	SELECT id 
	FROM lexems_sec
	ORDER BY id
	LIMIT 1 OFFSET num - 1;
$$ LANGUAGE SQL;

CREATE FUNCTION get_resource_id_by_number(num bigint)
RETURNS bigint AS
$$
	SELECT id 
	FROM resources_sec
	ORDER BY id
	LIMIT 1 OFFSET num - 1;
$$ LANGUAGE SQL;

CREATE FUNCTION upload_header(
	rmethod character varying(20), 
	rdomain character varying(50)	
) RETURNS SETOF int AS
$$
DECLARE mid int;
DECLARE did int;
BEGIN
	INSERT INTO methods_sec(name) VALUES(rmethod) 
		ON CONFLICT(name) DO NOTHING;
	INSERT INTO domains_sec(name) VALUES(rdomain) 
		ON CONFLICT(name) DO NOTHING;
	SELECT id FROM methods_sec WHERE name = rmethod INTO mid;
	SELECT id FROM domains_sec WHERE name = rdomain INTO did;

	INSERT INTO resources_headers_sec(method_id, domain_id) VALUES(mid, did)
		ON CONFLICT(method_id, domain_id) DO NOTHING;

	RETURN QUERY
	SELECT id FROM resources_headers_sec WHERE method_id = mid AND domain_id = did;
END
$$ LANGUAGE plpgsql;

CREATE FUNCTION upload_resource(
	rmethod character varying(20), 
	rdomain character varying(50),
	rname character varying(500),
    rtitle text,
	rcompression_type character varying(10),
	rcontent bytea,
	rsize int
) 
RETURNS TABLE(id bigint, upd boolean) AS
$$
DECLARE rh_id int;
DECLARE upd boolean := false;
BEGIN
	SELECT upload_header(rmethod, rdomain) INTO rh_id;
	SELECT 1 FROM resources_sec WHERE resource_header_id = rh_id AND name = rname INTO upd;
	INSERT INTO resources_sec(resource_header_id, name, title, compression_type, content, size) 
		VALUES(rh_id, rname, rtitle, rcompression_type, rcontent, rsize) 
		ON CONFLICT(resource_header_id, name) DO NOTHING;
    RETURN QUERY
	SELECT rs.id, COALESCE(upd, false) 
	FROM resources_sec AS rs 
	WHERE rs.resource_header_id = rh_id AND rs.name = rname;
END
$$ LANGUAGE plpgsql;

CREATE PROCEDURE upload_lexems(lexs new_lexem_type[]) AS
$$
    INSERT INTO lexems_sec(name, lang)
    SELECT * FROM UNNEST(lexs)
    ON CONFLICT(name, lang) DO NOTHING;
 
    UPDATE lexems_sec
    SET df = df + 1
	WHERE row(name, lang)::new_lexem_type = ANY(lexs);
$$ LANGUAGE SQL;

CREATE PROCEDURE upload_lexems_entries(resource_id bigint, entries new_lexem_entry_type[]) AS
$$
DECLARE new_lexems new_lexem_type[];
BEGIN
	SELECT ARRAY(
		SELECT row(name, lang)::new_lexem_type FROM UNNEST(entries)
	) INTO new_lexems;
	CALL upload_lexems(new_lexems);
    INSERT INTO resources_lexems_sec(res_id, lexem_id, wlf)
	SELECT resource_id, l.id, e.wlf
	FROM UNNEST(entries) AS e
	INNER JOIN lexems_sec AS l ON l.name = e.name AND l.lang = e.lang
	ON CONFLICT(res_id, lexem_id) DO UPDATE 
	SET wlf = excluded.wlf;
END
$$ LANGUAGE plpgsql;

CREATE PROCEDURE estimate_word_ranks() AS
$$
	WITH word_absolute_ranks AS (
		SELECT 
			rls.res_id, 
			rls.lexem_id,
			rls.wlf::real * LOG((SELECT COUNT(*) FROM resources_sec)::real / ls.df::real) AS value
		FROM resources_lexems_sec rls
		INNER JOIN lexems_sec ls ON rls.lexem_id = ls.id
		GROUP BY rls.res_id, rls.lexem_id, ls.df
	)
	UPDATE resources_lexems_sec AS rls
	SET rank = (
		SELECT value 
		FROM word_absolute_ranks war 
		WHERE rls.res_id = war.res_id AND rls.lexem_id = war.lexem_id
	);
	WITH resource_absolute_weights AS (
		SELECT 
			rls.res_id,
			SQRT(SUM(POWER(rls.rank, 2))) AS value
		FROM resources_lexems_sec rls
		GROUP BY rls.res_id
	)	
	UPDATE resources_lexems_sec AS rls
	SET rank = rank / (
		SELECT value 
		FROM resource_absolute_weights raw 
		WHERE rls.res_id = raw.res_id
	);
$$ LANGUAGE SQL;

CREATE PROCEDURE create_lexem_champion_list(lex_num int, size int, rank_threshold real) AS
$$
DECLARE lex_id int;
BEGIN
	SELECT get_lexem_id_by_number(lex_num) INTO lex_id;
	WITH top_ranked_resources AS (
		SELECT res_id 
		FROM resources_lexems_sec
		WHERE lexem_id = lex_id AND rank >= rank_threshold
		ORDER BY rank DESC
		LIMIT size
	)
	INSERT INTO champion_lists_sec VALUES
	(lex_id, ARRAY(SELECT res_id FROM top_ranked_resources));
END;
$$ LANGUAGE plpgsql;

CREATE FUNCTION get_resource_sec(num bigint) 
RETURNS resources_sec_named_view AS
$$
	SELECT *
	FROM resources_sec_named_view AS rs
	WHERE rs.id = (SELECT get_resource_id_by_number(num));
$$ LANGUAGE SQL;

CREATE FUNCTION upload_outcoming_resources(res_num bigint, urls text[])
RETURNS int AS 
$$
DECLARE res_id bigint;
BEGIN
	SELECT get_resource_id_by_number(res_num) INTO res_id;

	WITH outcoming_resources AS
	(
		SELECT id
		FROM resources_sec_named_view AS rsnv 
		WHERE (method || '://' || domain || name) IN (SELECT * FROM UNNEST(urls))
	)

	INSERT INTO resources_adjacency_sec(id_from, id_to)
	SELECT *
	FROM (SELECT res_id) as from_id, (SELECT * FROM outcoming_resources) AS to_id;

	RETURN (
		SELECT COUNT(*)
		FROM resources_adjacency_sec
		WHERE id_from = res_id
	);
END;
$$ LANGUAGE plpgsql;

CREATE PROCEDURE set_outcoming_resources_probability(res_num bigint, prob real) AS
$$
	UPDATE resources_adjacency_sec
	SET probability = prob
	WHERE id_from = (SELECT get_resource_id_by_number(res_num));
$$ LANGUAGE SQL;
 
CREATE PROCEDURE prepare_for_ranking_estimation() AS
$$
	UPDATE resources_sec
	SET rank = 1.0
	WHERE id = 1;

	INSERT INTO temporary_resource_ranks_sec(res_id)
	SELECT id FROM resources_sec
$$ LANGUAGE SQL;

CREATE PROCEDURE commit_resources_ranks() AS
$$
	UPDATE resources_sec AS rs
	SET rank = trrs.rank
	FROM temporary_resource_ranks_sec AS trrs
	WHERE rs.id = trrs.res_id;
$$ LANGUAGE SQL;

CREATE PROCEDURE update_resource_rank(res_num bigint, tp_probability real) AS
$$
DECLARE resource_id bigint;
DECLARE new_rank real;
BEGIN
	SELECT get_resource_id_by_number(res_num) INTO resource_id;

	WITH outcoming_resources AS
	(
		SELECT id_from AS id, probability
		FROM resources_adjacency_sec AS ras
		WHERE id_to = resource_id
	)

	SELECT SUM(p1 * COALESCE(p2, tp_probability))
	FROM (
		SELECT 
			rs.rank AS p1,
			outr.probability AS p2
		FROM resources_sec AS rs
		LEFT JOIN (SELECT * FROM outcoming_resources) AS outr ON outr.id = rs.id
	) AS tmp INTO new_rank;

	UPDATE temporary_resource_ranks_sec AS trrs
	SET rank = new_rank
	WHERE trrs.res_id = resource_id;
END;
$$ LANGUAGE plpgsql;