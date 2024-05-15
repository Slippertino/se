CREATE TABLE IF NOT EXISTS public.methods
(
    id int NOT NULL,
    name character varying(20) COLLATE pg_catalog."default" NOT NULL,
    CONSTRAINT pk_method_id PRIMARY KEY (id),
    CONSTRAINT uk_method_name UNIQUE (name)
);
    
CREATE TABLE IF NOT EXISTS public.domains
(
    id int NOT NULL,
    name character varying(50) COLLATE pg_catalog."default" NOT NULL,
    CONSTRAINT pk_domain_id PRIMARY KEY (id),
    CONSTRAINT uk_domain_name UNIQUE (name)
);

CREATE TABLE IF NOT EXISTS public.resources_headers
(
    id int NOT NULL,
	method_id int NOT NULL,
    domain_id int NOT NULL,
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
);

CREATE TABLE IF NOT EXISTS public.resources
(
	id bigint NOT NULL,
	resource_header_id int NOT NULL,
    name character varying(500) COLLATE pg_catalog."default" NOT NULL,
	title text NOT NULL,
	compression_type character varying(10) DEFAULT '',
	content bytea NOT NULL,
	size int NOT NULL,
	rank real NOT NULL DEFAULT 0.0,
    CONSTRAINT pk_resource PRIMARY KEY (id),
	CONSTRAINT uk_resource UNIQUE(resource_header_id, name),
    CONSTRAINT fk_resource_resources_headers FOREIGN KEY (resource_header_id)
        REFERENCES public.resources_headers (id) MATCH SIMPLE
        ON UPDATE CASCADE
        ON DELETE CASCADE
);

CREATE TABLE IF NOT EXISTS public.lexems
(
	id int NOT NULL,
	name character varying(300) NOT NULL,
	lang character varying(20) NOT NULL,
	df bigint NOT NULL DEFAULT 0,
	CONSTRAINT pk_lexems PRIMARY KEY(id),
	CONSTRAINT uk_lexems UNIQUE(name, lang)
);

CREATE TABLE IF NOT EXISTS public.resources_lexems
(
	res_id bigint NOT NULL,
	lexem_id int NOT NULL,
    wlf real NOT NULL,
	rank real,
	CONSTRAINT pk_resources_lexems PRIMARY KEY(res_id, lexem_id),
	CONSTRAINT fk_resources_lexems_reources FOREIGN KEY(res_id)
	    REFERENCES public.resources (id) MATCH SIMPLE
        ON UPDATE CASCADE
        ON DELETE CASCADE,
	CONSTRAINT fk_resources_lexems_lexems FOREIGN KEY(lexem_id)
	    REFERENCES public.lexems (id) MATCH SIMPLE
        ON UPDATE CASCADE
        ON DELETE CASCADE
);

CREATE TABLE IF NOT EXISTS public.champion_lists
(
	lexem_id int NOT NULL,
	resources_list bigint[] NOT NULL,
	CONSTRAINT pk_champion_lists PRIMARY KEY(lexem_id),
	CONSTRAINT fk_champion_lists_lexems FOREIGN KEY(lexem_id)
	    REFERENCES public.lexems (id) MATCH SIMPLE
        ON UPDATE CASCADE
        ON DELETE CASCADE
);

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