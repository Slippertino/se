CREATE EXTENSION IF NOT EXISTS "uuid-ossp";

CREATE TABLE IF NOT EXISTS public.methods
(
    id uuid NOT NULL DEFAULT uuid_generate_v1(),
    name character varying(20) COLLATE pg_catalog."default" NOT NULL,
    CONSTRAINT pk_method_id PRIMARY KEY (id),
    CONSTRAINT uk_method_name UNIQUE (name)
)

TABLESPACE pg_default;

ALTER TABLE IF EXISTS public.methods
    OWNER to admin;
    
CREATE TABLE IF NOT EXISTS public.domains
(
    id uuid NOT NULL DEFAULT uuid_generate_v1(),
    name character varying(50) COLLATE pg_catalog."default" NOT NULL,
    CONSTRAINT pk_domain_id PRIMARY KEY (id),
    CONSTRAINT uk_domain_name UNIQUE (name)
)

TABLESPACE pg_default;

ALTER TABLE IF EXISTS public.domains
    OWNER to admin;


CREATE TABLE IF NOT EXISTS public.resources_headers
(
    id uuid NOT NULL DEFAULT uuid_generate_v1(),
	method_id uuid NOT NULL,
    domain_id uuid NOT NULL,
    CONSTRAINT pk_resources_headers_id PRIMARY KEY (id),
	CONSTRAINT uk_resources_headers_id UNIQUE(method_id, domain_id),
	CONSTRAINT fk_resources_headers_method FOREIGN KEY (method_id)
        REFERENCES public.methods (id) MATCH SIMPLE
        ON UPDATE CASCADE
        ON DELETE CASCADE,
    CONSTRAINT fk_resources_headers_domain FOREIGN KEY (domain_id)
        REFERENCES public.domains (id) MATCH SIMPLE
        ON UPDATE CASCADE
        ON DELETE CASCADE
)

TABLESPACE pg_default;

ALTER TABLE IF EXISTS public.resources_headers
    OWNER to admin;

CREATE TABLE IF NOT EXISTS public.resources
(
	resource_header_id uuid NOT NULL,
    name character varying(500) COLLATE pg_catalog."default" NOT NULL,
	priority real NOT NULL,
    checksum character varying(64) COLLATE pg_catalog."default" NOT NULL,
    fin boolean NOT NULL,
    CONSTRAINT pk_resource PRIMARY KEY (resource_header_id, name),
    CONSTRAINT fk_resource_resources_headers FOREIGN KEY (resource_header_id)
        REFERENCES public.resources_headers (id) MATCH SIMPLE
        ON UPDATE CASCADE
        ON DELETE CASCADE
)

TABLESPACE pg_default;

ALTER TABLE IF EXISTS public.resources
    OWNER to admin;

CREATE VIEW resources_named_view AS
	SELECT 
		m.name AS method_name, 
		d.name AS domain_name, 
		r.name AS resource_name,
		r.priority,
		r.checksum,
		r.fin
	FROM resources AS r
	INNER JOIN resources_headers AS rh ON rh.id = r.resource_header_id
	INNER JOIN methods AS m ON m.id = rh.method_id
	INNER JOIN domains AS d ON d.id = rh.domain_id;

CREATE VIEW unhandled_resources_rate_view AS
	SELECT
		m.name AS method_name, 
		d.name AS domain_name, 
		COUNT(*) AS count
	FROM resources_headers AS rh
	INNER JOIN methods AS m ON m.id = rh.method_id
	INNER JOIN domains AS d ON d.id = rh.domain_id
	LEFT JOIN resources AS r ON r.resource_header_id = rh.id
	WHERE fin = false OR fin is null
	GROUP BY m.name, d.name
	ORDER BY count DESC;

CREATE FUNCTION upload_header(
	rmethod character varying(20), 
	rdomain character varying(50)	
) RETURNS SETOF uuid AS
$$
DECLARE mid uuid;
DECLARE did uuid;
BEGIN
	INSERT INTO methods(name) VALUES(rmethod) 
		ON CONFLICT(name) DO NOTHING;
	INSERT INTO domains(name) VALUES(rdomain) 
		ON CONFLICT(name) DO NOTHING;
	SELECT id FROM methods WHERE name = rmethod INTO mid;
	SELECT id FROM domains WHERE name = rdomain INTO did;

	INSERT INTO resources_headers(method_id, domain_id) VALUES(mid, did)
		ON CONFLICT(method_id, domain_id) DO NOTHING;

	RETURN QUERY
	SELECT id FROM resources_headers WHERE method_id = mid AND domain_id = did;
END
$$ LANGUAGE plpgsql;

CREATE PROCEDURE upload_resource(
	rmethod character varying(20), 
	rdomain character varying(50),
	rname character varying(500),
	rpriority real,
	rchecksum character varying(64)
) LANGUAGE plpgsql AS 
$$
DECLARE rh_id uuid;
BEGIN
	SELECT upload_header(rmethod, rdomain) INTO rh_id;
	INSERT INTO resources VALUES(rh_id, rname, rpriority, rchecksum, true) 
		ON CONFLICT(resource_header_id, name)
		DO UPDATE SET checksum = rchecksum, priority = rpriority, fin = true;
END
$$;

CREATE PROCEDURE reset() AS 
$$
	UPDATE resources
	SET fin = false;
$$ LANGUAGE SQL;

CREATE FUNCTION get_resource (
	rmethod character varying(20), 
	rdomain character varying(50),
	rname character varying(500)
) RETURNS SETOF resources_named_view AS 
$$
	SELECT * FROM resources_named_view 
	WHERE method_name = rmethod AND domain_name = rdomain AND resource_name = rname
$$ LANGUAGE SQL;

CREATE FUNCTION get_unhandled_domains_top(int) 
RETURNS SETOF unhandled_resources_rate_view AS
$$
	SELECT * FROM unhandled_resources_rate_view
	LIMIT $1;
$$ LANGUAGE SQL;

CREATE FUNCTION get_unhandled_resources_top_by_domain(
	rmethod character varying(20), 
	rdomain character varying(50),
	count integer
) RETURNS TABLE(
	name character varying(500), 
	priority real,
	checksum character varying(64)
) AS $$
	SELECT 
		resource_name,
		priority,
		checksum
	FROM resources_named_view 
	WHERE method_name = rmethod AND domain_name = rdomain AND fin = false
	ORDER BY priority DESC
	LIMIT count;
$$ LANGUAGE SQL;